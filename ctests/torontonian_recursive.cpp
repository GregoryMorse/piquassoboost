
#include <vector>

#include <random>
#include <chrono>
#include <string>

#include "TorontonianUtilities.h"
#include "Torontonian.h"
#include "TorontonianRecursive.h"


#include "matrix32.h"
#include "matrix.h"

#include "dot.h"

#include "matrix_helper.hpp"
#include "constants_tests.h"

#ifdef __MPI__
#include <mpi.h>
#endif // MPI


template<class matrix_type, class complex_type>
matrix_type
get_random_density_matrix(size_t dim){
    matrix_type posdef = pic::getRandomMatrix<matrix_type, complex_type>(dim, pic::POSITIVE_DEFINIT);
    matrix_type posdef_inverse = pic::calc_inverse_of_matrix<matrix_type, complex_type>(posdef);
    return posdef_inverse;
}


/**
@brief Unit test to compare torontonian calculators implemented in piqausso boost
*/
int main(){


    constexpr size_t dim = 34;

#ifdef __MPI__
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
#endif


    // create random matrix to calculate the torontonian
    pic::matrix mtx = get_random_density_matrix<pic::matrix, pic::Complex16>(dim);


#ifdef __MPI__
    // ensure that each MPI process gets the same input matrix from rank 0
    void* syncronized_data = (void*)mtx.get_data();
    MPI_Bcast(syncronized_data, mtx.size()*2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif


    // create class instance for torontonian calculator
    tbb::tick_count t0 = tbb::tick_count::now();
    pic::Torontonian torontonian_calculator(mtx);
    double result = torontonian_calculator.calculate();
    tbb::tick_count t1 = tbb::tick_count::now();

    std::cout << "torontonian calculator: " << result << std::endl;


    // create class instance for recursive torontonian calculator
    tbb::tick_count t2 = tbb::tick_count::now();
    pic::TorontonianRecursive recursive_torontonian_calculator(mtx);
    double result_recursive_extended = recursive_torontonian_calculator.calculate(true);
    tbb::tick_count t3 = tbb::tick_count::now();

    std::cout << "recursive torontonian calculator extended precision: " << result_recursive_extended<< std::endl;


    // create class instance for recursive torontonian calculator
    tbb::tick_count t4 = tbb::tick_count::now();
    recursive_torontonian_calculator = pic::TorontonianRecursive(mtx);
    double result_recursive_basic = 0;//recursive_torontonian_calculator.calculate(false);
    tbb::tick_count t5 = tbb::tick_count::now();

    std::cout << "recursive torontonian calculator basic precision: " << result_recursive_basic<< std::endl;

    std::cout << (t1-t0).seconds() << " " << (t3-t2).seconds() << " " << (t1-t0).seconds()/(t3-t2).seconds() << std::endl;


    return 0;

}
