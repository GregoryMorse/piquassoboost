#include <iostream>
#include "PowerTraceHafnian.h"
#include "dot.h"
#include <tbb/scalable_allocator.h>
#include "tbb/tbb.h"
#include <math.h>

#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
#pragma GCC target("avx")  //Enable AVX
#include <x86intrin.h> //AVX/SSE Extensions

extern "C" {

#define LAPACK_ROW_MAJOR               101

/// Definition of the LAPACKE_zgehrd function from LAPACKE to calculate the upper triangle hessenberg transformation of a matrix
int LAPACKE_zgehrd( int matrix_layout, int n, int ilo, int ihi, pic::Complex16* a, int lda, pic::Complex16* tau );

}


tbb::spin_mutex my_mutex;

double time_nominator = 0.0;
double time_nevezo = 0.0;


namespace pic {

typedef unsigned char Byte;



/**
@brief Calculates the n-th power of 2.
@param n An natural number
@return Returns with the n-th power of 2.
*/
unsigned long long power_of_2(unsigned long long n) {
  if (n == 0) return 1;
  if (n == 1) return 2;

  return 2 * power_of_2(n-1);
}








/**
@brief Default constructor of the class.
@param mtx_in The effective scattering matrix of a boson sampling instance
@return Returns with the instance of the class.
*/
PowerTraceHafnian::PowerTraceHafnian( matrix &mtx_in ) {

    mtx = mtx_in;

}




/**
@brief Call to calculate the hafnian of a complex matrix
@param mtx The matrix
@return Returns with the calculated hafnian
*/
Complex16
PowerTraceHafnian::calculate() {


    if ( mtx.rows != mtx.cols) {
        std::cout << "The input matrix should be square shaped, bu matrix with " << mtx.rows << " rows and with " << mtx.cols << " columns was given" << std::endl;
        std::cout << "Returning zero" << std::endl;
        return Complex16(0,0);
    }

#if BLAS==0 // undefined BLAS
    int NumThreads = omp_get_max_threads();
    omp_set_num_threads(1);
#elif BLAS==1 // MKL
    int NumThreads = mkl_get_max_threads();
    MKL_Set_Num_Threads(1);
#elif BLAS==2 //OpenBLAS
    int NumThreads = openblas_get_num_threads();
    openblas_set_num_threads(1);
#endif

    size_t dim = mtx.rows;


    if (mtx.rows == 0) {
        // the hafnian of an empty matrix is 1 by definition
        return Complex16(1,0);
    }
    else if (mtx.rows % 2 != 0) {
        // the hafnian of odd shaped matrix is 0 by definition
        return Complex16(0.0, 0.0);
    }


    size_t dim_over_2 = mtx.rows / 2;
    unsigned long long permutation_idx_max = power_of_2( (unsigned long long) dim_over_2);


    // for cycle over the permutations n/2 according to Eq (3.24) in arXiv 1805.12498
    tbb::combinable<Complex16> summands{[](){return Complex16(0.0,0.0);}};

    tbb::parallel_for( tbb::blocked_range<unsigned long long>(0, permutation_idx_max, 1), [&](tbb::blocked_range<unsigned long long> r ) {


        Complex16 &summand = summands.local();

        for ( unsigned long long permutation_idx=r.begin(); permutation_idx != r.end(); permutation_idx++) {

/*
    Complex16 summand(0.0,0.0);

    for (unsigned long long permutation_idx = 0; permutation_idx < permutation_idx_max; permutation_idx++) {
*/



        // get the binary representation of permutation_idx
        // also get the number of 1's in the representation and their position as 2*i and 2*i+1 in consecutive slots of the vector bin_rep
        std::vector<unsigned char> bin_rep;
        std::vector<unsigned char> positions_of_ones;
        bin_rep.reserve(dim_over_2);
        positions_of_ones.reserve(dim);
        for (int i = 1 << (dim_over_2-1); i > 0; i = i / 2) {
            if (permutation_idx & i) {
                bin_rep.push_back(1);
                positions_of_ones.push_back((bin_rep.size()-1)*2);
                positions_of_ones.push_back((bin_rep.size()-1)*2+1);
            }
            else {
                bin_rep.push_back(0);
            }
        }
        size_t number_of_ones = positions_of_ones.size();


        // matrix B corresponds to A^(Z), i.e. to the square matrix constructed from
        // the elements of mtx=A indexed by the rows and colums, where the binary representation of
        // permutation_idx was 1
        // for details see the text below Eq.(3.20) of arXiv 1805.12498
        matrix B(number_of_ones, number_of_ones);
        for (size_t idx = 0; idx < number_of_ones; idx++) {
            for (size_t jdx = 0; jdx < number_of_ones; jdx++) {
                B[idx*number_of_ones + jdx] = mtx[positions_of_ones[idx]*dim + ((positions_of_ones[jdx]) ^ 1)];
            }
        }


        // calculating Tr(B^j) for all j's that are 1<=j<=dim/2
        // this is needed to calculate f_G(Z) defined in Eq. (3.17b) of arXiv 1805.12498
        matrix traces(dim_over_2, 1);
        if (number_of_ones != 0) {
            traces = calc_power_traces(B, dim_over_2);
        }
        else{
            // in case we have no 1's in the binary representation of permutation_idx we get zeros
            // this occurs once during the calculations
            memset( traces.get_data(), 0.0, traces.rows*traces.cols*sizeof(Complex16));
        }


        // fact corresponds to the (-1)^{(n/2) - |Z|} prefactor from Eq (3.24) in arXiv 1805.12498
        bool fact = ((dim_over_2 - number_of_ones/2) % 2);


        // auxiliary data arrays to evaluate the second part of Eqs (3.24) and (3.21) in arXiv 1805.12498
        matrix aux0(dim_over_2 + 1, 1);
        matrix aux1(dim_over_2 + 1, 1);
        memset( aux0.get_data(), 0.0, (dim_over_2 + 1)*sizeof(Complex16));
        memset( aux1.get_data(), 0.0, (dim_over_2 + 1)*sizeof(Complex16));
        aux0[0] = 1.0;
        // pointers to the auxiliary data arrays
        Complex16 *p_aux0=NULL, *p_aux1=NULL;

        matrix comb(2 * (dim_over_2 + 1),1);
        memset( comb.get_data(), 0.0, comb.rows*comb.cols*sizeof(Complex16));
        comb[0] = 1.0;

        for (size_t idx = 1; idx <= dim_over_2; idx++) {


            Complex16 factor = traces[idx - 1] / (2.0 * idx);
            Complex16 powfactor(1.0,0.0);



            if (idx%2 == 1) {
                p_aux0 = aux0.get_data();
                p_aux1 = aux1.get_data();
            }
            else {
                p_aux0 = aux1.get_data();
                p_aux1 = aux0.get_data();
            }

            memcpy(p_aux1, p_aux0, (dim_over_2+1)*sizeof(Complex16) );

            for (size_t jdx = 1; jdx <= (dim / (2 * idx)); jdx++) {
                powfactor = powfactor * factor / ((double)jdx);

                for (size_t kdx = idx * jdx + 1; kdx <= dim_over_2 + 1; kdx++) {
                    p_aux1[kdx-1] += p_aux0[kdx-idx*jdx - 1]*powfactor;
                }



            }



        }


        if (fact) {
            summand = summand - p_aux1[dim_over_2];
        }
        else {
            summand = summand + p_aux1[dim_over_2];
        }

        }

    });

    // the resulting Hafnian of matrix mat
    Complex16 res(0,0);
    summands.combine_each([&res](Complex16 a) {
        res = res + a;
    });

    //Complex16 res = summand;


#if BLAS==0 // undefined BLAS
    omp_set_num_threads(NumThreads);
#elif BLAS==1 //MKL
    MKL_Set_Num_Threads(NumThreads);
#elif BLAS==2 //OpenBLAS
    openblas_set_num_threads(NumThreads);
#endif


    return res;
}



/**
@brief Call to calculate the power traces \f$Tr(mtx^j)~\forall~1\leq j\leq l\f$ for a squared complex matrix \f$mtx\f$ of dimensions \f$n\times n\f$.
@param mtx an instance of class matrix.
@param pow_max maximum matrix power when calculating the power trace.
@return a vector containing the power traces of matrix `z` to power \f$1\leq j \leq l\f$.
*/
matrix
PowerTraceHafnian::calc_power_traces(matrix &mtx, size_t pow_max) {

  // transform the matrix mtx into an upper hessenberg format by calling lapack function
  int N = mtx.rows;
  int ILO = 1;
  int IHI = N;
  int LDA = N;
  matrix tau(N-1,1);
  LAPACKE_zgehrd(LAPACK_ROW_MAJOR, N, ILO, IHI, mtx.get_data(), LDA, tau.get_data() );

  // calculate the coefficients of the characteristic polynomiam by LaBudde algorithm
  matrix coeffs_labudde = calc_characteristic_polynomial_coeffs(mtx, mtx.rows);

  // calculate the power traces of the matrix mtx using LeVerrier recursion relation
  return powtrace_from_charpoly(coeffs_labudde, pow_max);
}




/**
@brief Call to determine the first \f$ k \f$ coefficients of the characteristic polynomial using the Algorithm 2 of LaBudde method.
 See [arXiv:1104.3769](https://arxiv.org/abs/1104.3769v1) for further details.
@param mtx matrix in upper Hessenberg form.
@param highest_orde the order of the highest order coefficient to be calculated (k <= n)
@return Returns with the calculated coefficients of the characteristic polynomial.
 *
 */
matrix
PowerTraceHafnian::calc_characteristic_polynomial_coeffs(matrix &mtx, size_t highest_order)
{
 // the matrix c holds not just the polynomial coefficients, but also auxiliary
 // data. To retrieve the characteristic polynomial coeffients from the matrix c, use
 // this map for characteristic polynomial coefficient c_j:
 // if j = 0, c_0 -> 1
 // if j > 0, c_j -> c[(n - 1) * n + j - 1]


    // check the dimensions of the matrix in debug mode
    assert( mtx.rows == mtx.cols);

    //dimension of the matrix
    size_t dim = mtx.rows;


    // allocate memory for the coefficients c_k of p(\lambda)
    matrix coeffs(dim, dim);
    memset(coeffs.get_data(), 0, dim*dim*sizeof(Complex16));


    // c^(1)_1 = -\alpha_1
    coeffs[0] = -mtx[0];

    // c^(2)_1 = c^(1)_1 - \alpha_2
    coeffs[dim] = coeffs[0] - mtx[dim+1];

    // c^(2)_2 = \alpha_1\alpha_2 - h_{12}\beta_2
    coeffs[dim+1] =  mtx[0]*mtx[dim+1] - mtx[1]*mtx[dim];

    // for (i=3:k do)
    for (size_t idx=2; idx<=highest_order-1; idx++) {
        // i = idx + 1

        // calculate the products of matrix elements \beta_i
        // the n-th (0<=n<=idx-2) element of the arary stands for:
        // beta_prods[n] = \beta_i * \beta_i-1 * ... * \beta_{i-n}
        matrix beta_prods(idx,1);
        beta_prods[0] = mtx[idx*dim + idx-1];
        for (size_t prod_idx=1; prod_idx<=idx-1; prod_idx++) {
            beta_prods[prod_idx] = beta_prods[prod_idx-1] * mtx[(idx-prod_idx)*dim + (idx-prod_idx-1)];
        }

        // c^(i)_1 = c^(i-1)_1 - \alpha_i
        coeffs[idx*dim] = coeffs[(idx-1)*dim] - mtx[idx*dim + idx];

        // for j=2 : i-1 do
        for (size_t jdx=1; jdx<=idx-1; jdx++) {
            // j = jdx + 1

            // sum = \sum_^{j-2}{m=1} h_{i-m,i} \beta_i*...*\beta_{i-m+1} c^{(i-m-1)}_{j-m-1}  - h_{i-j+1,i}* beta_i*...*beta_{i-j+2}
            Complex16 sum(0.0,0.0);

            // for m=j-2 : 1 do
            for ( size_t mdx=1; mdx<=jdx-1; mdx++) {
                // m = mdx

                // sum = sum + h_{i-m, i} * beta_prod * c^{(i-m-1)}_{j-m-1}
                sum = sum + mtx[(idx-mdx)*dim+idx] * beta_prods[mdx-1] * coeffs[(idx-mdx-1)*dim + jdx-mdx-1];

            }

            // sum = sum + h_{i-j+1,i} * \beta_prod
            sum = sum + mtx[(idx-jdx)*dim + idx] * beta_prods[jdx-1];

            // c^(i)_j = c^(i-1)_j - \alpha_i*c^(i-1)_{j-1} - sum
            coeffs[idx*dim+jdx] = coeffs[(idx-1)*dim+jdx] - mtx[idx*dim+idx] * coeffs[(idx-1)*dim + jdx-1] - sum;
        }

        // sum = \sum_^{i-2}{m=1} h_{i-m,i} \beta_i*...*\beta_{i-m+1} c^{(i-m-1)}_{i-m-1}  - h_{1,i}* beta_i*...*beta_{2}
        Complex16 sum(0.0,0.0);

        // for m=j-2 : 1 do
        for ( size_t mdx=1; mdx<=idx-1; mdx++) {
            // m = mdx

            // sum = sum + h_{i-m, i} * beta_prod * c^{(i-m-1)}_{j-m-1}
            sum = sum + mtx[(idx-mdx)*dim+idx] * beta_prods[mdx-1] * coeffs[(idx-mdx-1)*dim + idx-mdx-1];
        }

        // c^(i)_i = -\alpha_i c^{(i-1)}_{i-1} - sum
        coeffs[idx*dim+idx] = -mtx[idx*dim+idx]*coeffs[(idx-1)*dim+idx-1] - sum - mtx[idx]*beta_prods[beta_prods.size()-1];

    }

    // for i=k+1 : n do
    for (size_t idx = highest_order; idx<dim; idx++ ) {
        // i = idx + 1

        // c^(i)_1 = c^(i-1)_1 - \alpha_i
        coeffs[idx*dim] = coeffs[(idx-1)*dim] - mtx[idx*dim + idx];

        // calculate the products of matrix elements \beta_i
        // the n-th (0<=n<=idx-2) element of the arary stands for:
        // beta_prods[n] = \beta_i * \beta_i-1 * ... * \beta_{i-n}

        if (highest_order >= 2) {
            matrix beta_prods(idx,1);
            beta_prods[0] = mtx[idx*dim + idx-1];
            for (size_t prod_idx=1; prod_idx<=idx-1; prod_idx++) {
                beta_prods[prod_idx] = beta_prods[prod_idx-1] * mtx[(idx-prod_idx)*dim + (idx-prod_idx-1)];
            }

            // for j = 2 : k do
            for (size_t jdx=1; jdx<=highest_order-1; jdx++) {
                // j = jdx + 1

                // sum = \sum_^{j-2}{m=1} h_{i-m,i} \beta_i*...*\beta_{i-m+1} c^{(i-m-1)}_{j-m-1}  - h_{i-j+1,i}* beta_i*...*beta_{i-j+2}
                Complex16 sum(0.0,0.0);

                // for m=j-2 : 1 do
                for ( size_t mdx=1; mdx<=jdx-1; mdx++) {
                    // m = mdx

                    // sum = sum + h_{i-m, i} * beta_prod * c^{(i-m-1)}_{j-m-1}
                    sum = sum + mtx[(idx-mdx)*dim+idx] * beta_prods[mdx-1] * coeffs[(idx-mdx-1)*dim + jdx-mdx-1];

                }

                // sum = sum + h_{i-j+1,i} * \beta_prod
                sum = sum + mtx[(idx-jdx)*dim + idx] * beta_prods[jdx-1];

                // c^(i)_j = c^(i-1)_j - \alpha_i*c^(i-1)_{j-1} - sum
                coeffs[idx*dim+jdx] = coeffs[(idx-1)*dim+jdx] - mtx[idx*dim+idx] * coeffs[(idx-1)*dim + jdx-1] - sum;
            }



         }
    }


  return coeffs;
}



/**
@brief Call to calculate the traces of \f$ A^{p}\f$, where 1<=p<=pow is an integer and A is a square matrix.
The trace is calculated from the coefficients of its characteristic polynomial.
In the case that the power p is above the size of the matrix we can use an optimization described in Appendix B of [arxiv:1805.12498](https://arxiv.org/pdf/1104.3769v1.pdf)
@param c matrix containing the characteristic polynomial coefficients
@param pow the maximal exponent p
@return Returns with the calculated power traces
 */
matrix
PowerTraceHafnian::powtrace_from_charpoly(matrix &coeffs, size_t pow) {

  size_t dim = coeffs.rows;


  if (pow == 0) {
    matrix ret(1,1);
    ret[0].real( (double) dim );
    ret[0].imag( 0.0 );
    return ret;
  }

  // allocate memory for the power traces
  matrix traces(pow,1);



  traces[0] = -coeffs[(dim - 1) * dim];

  // Calculate power traces using the LeVerrier recursion relation
  for (size_t idx = 2; idx <= pow; idx++) {

    size_t element_offset = (dim - 1) * dim + idx - 1;
    traces[idx - 1] = -(double)idx * coeffs[element_offset];

    for (size_t j = idx - 1; j >= 1; j--) {
      traces[idx - 1] -= coeffs[element_offset - j] * traces[j - 1];
    }

  }

  // Appendix B optimization
  if (pow > dim) {
    for (size_t idx = 1; idx <= pow - dim; idx++) {

      size_t element_offset = dim + idx - 1;
      size_t element_offset_coeffs = (dim - 1) * dim - 1;
      traces[element_offset] = 0.0;

      for (size_t jdx = 1; jdx <= dim; jdx++) {
        traces[element_offset] -= traces[element_offset - jdx] * coeffs[element_offset_coeffs + jdx];
      }

    }

  } // if

/*
        {
            tbb::spin_mutex::scoped_lock my_lock{my_mutex};
//std::cout <<   (t2-t1).seconds()/(t1-t0).seconds() << std::endl;
//time_nominator = time_nominator + (t2-t1).seconds();
//time_nevezo = time_nevezo + (t1-t0).seconds();
//std::cout <<   time_nominator/time_nevezo << std::endl;
  //traces2.print_matrix();
  //traces.print_matrix();
        }
*/

  return traces;

}





/**
@brief Call to update the memory address of the matrix mtx
@param mtx_in Input matrix defined by
*/
void
PowerTraceHafnian::Update_mtx( matrix &mtx_in) {

    mtx = mtx_in;

}



} // PIC