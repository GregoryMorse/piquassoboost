#ifndef PowerTraceLoopHafnian_H
#define PowerTraceLoopHafnian_H

#include "PowerTraceHafnian.h"


namespace pic {



/**
@brief Class to calculate the loop hafnian of a complex matrix by the power trace method
*/
class PowerTraceLoopHafnian : public PowerTraceHafnian{


public:


/**
@brief Default constructor of the class.
@return Returns with the instance of the class.
*/
PowerTraceLoopHafnian();

/**
@brief Constructor of the class.
@param mtx_in A symmetric matrix for which the hafnian is calculated. ( In GBS calculations the \f$ a_1, a_2, ... a_n, a_1^*, a_2^*, ... a_n^* \f$ ordered covariance matrix of the Gaussian state.)
@return Returns with the instance of the class.
*/
PowerTraceLoopHafnian( matrix &mtx_in );


/**
@brief Call to calculate the hafnian of a complex matrix stored in the instance of the class
@return Returns with the calculated loop hafnian
*/
Complex16 calculate();

/**
@brief Call to calculate the hafnian of a complex matrix stored in the instance of the class
@param start_idx The minimal index evaluated in the exponentially large sum (used to divide calculations between MPI processes)
@param step_idx The index step in the exponentially large sum (used to divide calculations between MPI processes)
@param max_idx The maximal indexe valuated in the exponentially large sum (used to divide calculations between MPI processes)
@return Returns with the calculated hafnian
*/
Complex16 calculate(unsigned long long start_idx, unsigned long long step_idx, unsigned long long max_idx );


/**
@brief Call to update the memory address of the matrix mtx
@param mtx_in A symmetric matrix for which the hafnian is calculated. (For example a covariance matrix of the Gaussian state.)
*/
void Update_mtx( matrix &mtx_in);

protected:

/**
@brief Call to scale the input matrix according to according to Eq (2.14) of in arXiv 1805.12498
*/
virtual void ScaleMatrix();





}; //PowerTraceLoopHafnian


} // PIC

#endif