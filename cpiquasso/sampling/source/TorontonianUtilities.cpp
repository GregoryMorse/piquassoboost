#include "TorontonianUtilities.hpp"
#include "calc_cholesky_decomposition_AVX.h"
#include <iostream>
#include "common_functionalities.h"
#include <math.h>

/*
static tbb::spin_mutex my_mutex;

double time_nominator = 0.0;
double time_nevezo = 0.0;
*/

namespace pic {


/**
@brief Call to calculate in-place Cholesky decomposition of a matrix. The decomposed matrix is stored in mtx.
@param mtx A positive definite hermitian matrix with eigenvalues less then unity.
*/
void
calc_cholesky_decomposition(matrix32& matrix)
{
    Complex32 determinant(0.0,0.0);
    calc_cholesky_decomposition( matrix, 0, determinant);
    return;
}


/**
@brief Call to calculate in-place Cholesky decomposition of a matrix
@param mtx A positive definite hermitian matrix with eigenvalues less then unity.  The decomposed matrix is stored in mtx.
@param reuse_index Labels the row and column from which the Cholesky decomposition should be continued.
*/
void
calc_cholesky_decomposition(matrix32& matrix, const size_t reuse_index)
{
    Complex32 determinant(0.0,0.0);
    calc_cholesky_decomposition( matrix, reuse_index, determinant);
    return;
}


/**
@brief Call to calculate in-place Cholesky decomposition of a matrix
@param mtx A positive definite hermitian matrix with eigenvalues less then unity.  The decomposed matrix is stored in mtx.
@param reuse_index Labels the row and column from which the Cholesky decomposition should be continued.
@param determinant The determinant of the matrix is calculated and stored in this variable.
(if reuse_index index is greater than 0, than the contributions of the first reuse_index-1 elements of the Cholesky L matrix should be multiplied manually)
*/
void
calc_cholesky_decomposition(matrix32& mtx, const size_t reuse_index, Complex32 &determinant)
{

    calc_cholesky_decomposition<matrix32, Complex32>(mtx, reuse_index, determinant);
    return;

}


/**
@brief Call to calculate in-place Cholesky decomposition of a matrix using the Lapack implementation
@param mtx A positive definite hermitian matrix with eigenvalues less then unity.  The decomposed matrix is stored in mtx.
*/
void
calc_cholesky_decomposition_lapack(matrix &matrix) {


// transform the matrix mtx into an upper Hessenberg format by calling lapack function
        char UPLO = 'L';
        int N = matrix.rows;
        int LDA = matrix.stride;

        //std::cout<<"Before lapacke call:\n";
        //mtx.print_matrix();


        LAPACKE_zpotrf(LAPACK_ROW_MAJOR, UPLO, N, matrix.get_data(), LDA);

        return;

}


/**
@brief Call to calculate the determinant of a matrix by Cholesky decomposition.
@param mtx A positive definite hermitian matrix with eigenvalues less then unity.  The decomposed matrix is stored in mtx.
@return Returns with the calculated determiant
*/
Complex32
calc_determinant_cholesky_decomposition(matrix32& mtx){

 return  calc_determinant_cholesky_decomposition(mtx, 0);

}


/**
@brief Call to calculate the determinant of a matrix by Cholesky decomposition.
@param mtx A positive definite hermitian matrix with eigenvalues less then unity.  The decomposed matrix is stored in mtx.
@param L A partial Cholesky decomposition of the matrix mtx. The first (reuse_index-1) diagonal elements are used to calculate the determinant.
@param reuse_index Labels the row and column from which the Cholesky decomposition should be continued.
@return Returns with the calculated determiant
*/
Complex32
calc_determinant_cholesky_decomposition(matrix32& mtx, const size_t reuse_index){

        Complex32 determinant(1.0, 0.0);

        // calculate the rest of the Cholesky decomposition and calculate the determinant
        calc_cholesky_decomposition(mtx, reuse_index, determinant);

#ifdef DEBUG
        if (reuse_index > mtx.rows ) {
            std::cout << "calc_determinant_cholesky_decomposition: reuse index should be smaller than the matrix size!" << std::endl;
            exit(-1);
        }

#endif

        // multiply the result with the remaining diagonal elements of the Cholesky matrix L, that has been reused

        for (size_t idx=0; idx < reuse_index; idx++){
                determinant *= mtx[idx * mtx.stride + idx];
        }


        return mult_a_bconj( determinant, determinant);

}






} // PIC


