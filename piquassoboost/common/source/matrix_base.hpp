/**
 * Copyright 2021 Budapest Quantum Computing Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef matrix_BASE_H
#define matrix_BASE_H

#include "PicTypes.hpp"
#include <cstring>
#include <iostream>
#include <tbb/scalable_allocator.h>
#include <tbb/spin_mutex.h>


/// The namespace of the Picasso project
namespace pic {





/**
@brief Base Class to store data of row-mayor arrays and its properties.
*/
template<typename scalar>
class matrix_base {

public:
  /// The number of rows
  size_t rows;
  /// The number of columns
  size_t cols;
  /// The column stride of the array. (The array elements in one row are a_0, a_1, ... a_{cols-1}, 0, 0, 0, 0. The number of zeros is stride-cols)
  size_t stride;
  /// pointer to the stored data
  scalar* data;

protected:

  /// logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  bool conjugated;
  /// logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  bool transposed;
  /// logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  bool owner;
  /// mutual exclusion to count the references for class instances referring to the same data.
  tbb::spin_mutex* reference_mutex;
  /// the number of the current references of the present object
  int64_t* references;



public:

/**
@brief Default constructor of the class.
@return Returns with the instance of the class.
*/
matrix_base() {

  // The number of rows
  rows = 0;
  // The number of columns
  cols = 0;
  // The column stride of the matrix
  stride = 0;
  // pointer to the stored data
  data = NULL;
  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  conjugated = false;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  transposed = false;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  owner = false;
  // mutual exclusion to count the references for class instances referring to the same data.
  reference_mutex = new tbb::spin_mutex();
  references = new int64_t;
  (*references)=1;
}


/**
@brief Constructor of the class. By default the created class instance would not be owner of the stored data.
@param data_in The pointer pointing to the data
@param rows_in The number of rows in the stored matrix
@param cols_in The number of columns in the stored matrix
@return Returns with the instance of the class.
*/
matrix_base( scalar* data_in, size_t rows_in, size_t cols_in) {

  // The number of rows
  rows = rows_in;
  // The number of columns
  cols = cols_in;
  // The column stride of the matrix
  stride = cols_in;
  // pointer to the stored data
  data = data_in;
  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  conjugated = false;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  transposed = false;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  owner = false;
  // mutual exclusion to count the references for class instances referring to the same data.
  reference_mutex = new tbb::spin_mutex();
  references = new int64_t;
  (*references)=1;
}



/**
@brief Constructor of the class. By default the created class instance would not be owner of the stored data.
@param data_in The pointer pointing to the data
@param rows_in The number of rows in the stored matrix
@param cols_in The number of columns in the stored matrix
@param stride_in The column stride of the matrix array (The array elements in one row are a_0, a_1, ... a_{cols-1}, 0, 0, 0, 0. The number of zeros is stride-cols)
@return Returns with the instance of the class.
*/
matrix_base( scalar* data_in, size_t rows_in, size_t cols_in, size_t stride_in) {

  // The number of rows
  rows = rows_in;
  // The number of columns
  cols = cols_in;
  // The column stride of the matrix
  stride = stride_in;
  // pointer to the stored data
  data = data_in;
  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  conjugated = false;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  transposed = false;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  owner = false;
  // mutual exclusion to count the references for class instances referring to the same data.
  reference_mutex = new tbb::spin_mutex();
  references = new int64_t;
  (*references)=1;
}



/**
@brief Constructor of the class. Allocates data for matrix rows_in times cols_in. By default the created instance would be the owner of the stored data.
@param rows_in The number of rows in the stored matrix
@param cols_in The number of columns in the stored matrix
@return Returns with the instance of the class.
*/
matrix_base( size_t rows_in, size_t cols_in) {

  // The number of rows
  rows = rows_in;
  // The number of columns
  cols = cols_in;
  // The column stride of the matrix
  stride = cols_in;
  // pointer to the stored data
  data = (scalar*)scalable_aligned_malloc( rows*cols*sizeof(scalar), CACHELINE);
#ifdef DEBUG
  if (rows > 0 && cols>0) assert(data);
#endif
  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  conjugated = false;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  transposed = false;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  owner = true;
  // mutual exclusion to count the references for class instances referring to the same data.
  reference_mutex = new tbb::spin_mutex();
  references = new int64_t;
  (*references)=1;


}



/**
@brief Constructor of the class. Allocates data for matrix rows_in times cols_in. By default the created instance would be the owner of the stored data.
@param rows_in The number of rows in the stored matrix
@param cols_in The number of columns in the stored matrix
@param stride_in The column stride of the matrix array (The array elements in one row are a_0, a_1, ... a_{cols-1}, 0, 0, 0, 0. The number of zeros is stride-cols)
@return Returns with the instance of the class.
*/
matrix_base( size_t rows_in, size_t cols_in, size_t stride_in) {

  // The number of rows
  rows = rows_in;
  // The number of columns
  cols = cols_in;
  // The column stride of the matrix
  stride = stride_in;
  // pointer to the stored data
  data = (scalar*)scalable_aligned_malloc( rows*stride*sizeof(scalar), CACHELINE);
#ifdef DEBUG
  if (rows > 0 && cols>0) assert(data);
#endif
  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  conjugated = false;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  transposed = false;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  owner = true;
  // mutual exclusion to count the references for class instances referring to the same data.
  reference_mutex = new tbb::spin_mutex();
  references = new int64_t;
  (*references)=1;


}


/**
@brief Copy constructor of the class. The new instance shares the stored memory with the input matrix. (Needed for TBB calls)
@param An instance of class matrix to be copied.
*/
matrix_base(const matrix_base<scalar> &in) {

    data = in.data;
    rows = in.rows;
    cols = in.cols;
    stride = in.stride;
    transposed = in.transposed;
    conjugated = in.conjugated;
    owner = in.owner;

    reference_mutex = in.reference_mutex;
      references = in.references;

    {
      tbb::spin_mutex::scoped_lock my_lock{*reference_mutex};
      (*references)++;
    }



}



/**
@brief Destructor of the class
*/
~matrix_base() {
  release_data();
}

/**
@brief Call to get whether the matrix should be conjugated in CBLAS functions or not.
@return Returns with true if the matrix should be conjugated in CBLAS functions or false otherwise.
*/
bool is_conjugated() {
  return conjugated;
}

/**
@brief Call to conjugate (or un-conjugate) the matrix for CBLAS functions.
*/
void conjugate() {

  conjugated = !conjugated;

}


/**
@brief Call to get whether the matrix should be conjugated in CBLAS functions or not.
@return Returns with true if the matrix should be conjugated in CBLAS functions or false otherwise.
*/
bool is_transposed() {

  return transposed;

}


/**
@brief Call to transpose (or un-transpose) the matrix for CBLAS functions.
*/
void transpose()  {

  transposed = !transposed;

}




/**
@brief Call to get the pointer to the stored data
*/
scalar* get_data() {

  return data;

}


/**
@brief Call to replace the stored data by an another data array. If the class was the owner of the original data array, then it is released.
@param data_in The data array to be set as a new storage.
@param owner_in Set true to set the current class instance to be the owner of the data array, or false otherwise.
*/
void replace_data( scalar* data_in, bool owner_in) {

    release_data();
    data = data_in;
    owner = owner_in;

    reference_mutex = new tbb::spin_mutex();
    references = new int64_t;
    (*references)=1;

}


/**
@brief Call to release the data stored by the matrix. (If the class instance was not the owner of the data, then the data pointer is simply set to NULL pointer.)
*/
void release_data() {

    if (references==NULL) return;
    bool call_delete = false;

{

    tbb::spin_mutex::scoped_lock my_lock{*reference_mutex};

    if (references==NULL) return;
    call_delete = ((*references)==1);


    if (call_delete) {
      // release the data when matrix is the owner
      if (owner) {
        scalable_aligned_free(data);
      }
      delete references;
    }
    else {
        (*references)--;
    }

    data = NULL;
    references = NULL;

}

  if ( call_delete && reference_mutex !=NULL) {
    delete reference_mutex;
  }

}



/**
@brief Call to set the current class instance to be (or not to be) the owner of the stored data array.
@param owner_in Set true to set the current class instance to be the owner of the data array, or false otherwise.
*/
void set_owner( bool owner_in)  {

    owner=owner_in;

}

/**
@brief Assignment operator.
@param mtx An instance of class matrix_base
@return Returns with the instance of the class.
*/
void operator= (const matrix_base& mtx ) {

  // releasing the containing data
  release_data();

  // The number of rows
  rows = mtx.rows;
  // The number of columns
  cols = mtx.cols;
  // The column stride of the matrix
  stride = mtx.stride;
  // pointer to the stored data
  data = mtx.data;
  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  conjugated = mtx.conjugated;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  transposed = mtx.transposed;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  owner = mtx.owner;

  reference_mutex = mtx.reference_mutex;
  references = mtx.references;

  {
      tbb::spin_mutex::scoped_lock my_lock{*reference_mutex};
      (*references)++;
  }

}


/**
@brief Operator [] to access elements in array style (checks the boundaries of the stored array in DEBUG target)
@param idx the index of the element
@return Returns with a reference to the idx-th element.
*/
scalar& operator[](size_t idx) {

#ifdef DEBUG
    if ( idx >= rows*stride || idx < 0) {
        std::cout << "Accessing element out of bonds. Exiting" << std::endl;
        exit(-1);
    }
#endif

    return data[idx];
}

/**
@brief Operator [] to access elements in array style (checks the boundaries of the stored array in DEBUG target)
@param idx the index of the element
@return Returns with a reference to the idx-th element.
*/
const scalar& operator[](size_t idx) const {

#ifdef DEBUG
    if ( idx >= rows*stride || idx < 0) {
        std::cout << "Accessing element out of bonds. Exiting" << std::endl;
        exit(-1);
    }
#endif

    return data[idx];
}






/**
@brief Call to create a copy of the matrix
@return Returns with the instance of the class.
*/
matrix_base<scalar> copy() const {

  matrix_base<scalar> ret = matrix_base<scalar>(rows, cols, stride);

  // logical variable indicating whether the matrix needs to be conjugated in CBLAS operations
  ret.conjugated = conjugated;
  // logical variable indicating whether the matrix needs to be transposed in CBLAS operations
  ret.transposed = transposed;
  // logical value indicating whether the class instance is the owner of the stored data or not. (If true, the data array is released in the destructor)
  ret.owner = true;

  memcpy( ret.data, data, rows*cols*sizeof(scalar));

  return ret;

}



/**
@brief Call to get the number of the allocated elements
@return Returns with the number of the allocated elements (rows*cols)
*/
size_t size() {

  return rows*cols;

}


/**
@brief Call to prints the stored matrix on the standard output
*/
void print_matrix() {
    std::cout << std::endl << "The stored matrix:" << std::endl;
    for ( size_t row_idx=0; row_idx < rows; row_idx++ ) {
        for ( size_t col_idx=0; col_idx < cols; col_idx++ ) {
            size_t element_idx = row_idx*stride + col_idx;
              std::cout << " " << data[element_idx];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl << std::endl;

}



/**
@brief Call to check the array for NaN entries.
@return Returns with true if the array has at least one NaN entry.
*/
bool
isnan() {
    for (size_t i = 0; i < rows * cols; i++) {
        if ( std::isnan(data[i]) ){
            return true;
        }
    }

    return false;
}




}; //matrix_base






}  //PIC

#endif
