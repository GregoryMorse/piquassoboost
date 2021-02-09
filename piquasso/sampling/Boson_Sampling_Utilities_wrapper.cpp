#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <numpy/arrayobject.h>
#include "structmember.h"
#include "CChinHuhPermanentCalculator.h"



/**
@brief Type definition of the ChinHuhPermanentCalculator_wrapper Python class of the ChinHuhPermanentCalculator_wrapper module
*/
typedef struct ChinHuhPermanentCalculator_wrapper {
    PyObject_HEAD
    /// pointer to numpy matrix to keep it alive
    PyObject *matrix = NULL;
    /// pointer to numpy matrix of input states to keep it alive
    PyObject *input_state = NULL;
    /// pointer to numpy matrix of output states to keep it alive
    PyObject *output_state = NULL;
    /// The C++ variant of class CChinHuhPermanentCalculator
    pic::CChinHuhPermanentCalculator* calculator;
} ChinHuhPermanentCalculator_wrapper;


/**
@brief Creates an instance of class ChinHuhPermanentCalculator and return with a pointer pointing to the class instance (C++ linking is needed)
@param C
@param G
@param mean
@return Return with a void pointer pointing to an instance of N_Qubit_Decomposition class.
*/
pic::CChinHuhPermanentCalculator* 
create_ChinHuhPermanentCalculator( pic::matrix &matrix_mtx, pic::PicState_int64 &input_state_mtx, pic::PicState_int64 &output_state_mtx ) {

    return new pic::CChinHuhPermanentCalculator(matrix_mtx, input_state_mtx, output_state_mtx);
}

/**
@brief Call to deallocate an instance of ChinHuhPermanentCalculator class
@param ptr A pointer pointing to an instance of ChinHuhPermanentCalculator class.
*/
void
release_ChinHuhPermanentCalculator( pic::CChinHuhPermanentCalculator*  instance ) {
    if ( instance != NULL ) {
        delete instance;
    }
    return;
}


/**
@brief Call to create a PIC matrix representation of a numpy array
*/
pic::matrix 
numpy2matrix(PyObject *arr) {

    // test C-style contiguous memory allocation of the arrays
    if ( !PyArray_IS_C_CONTIGUOUS(arr) ) {
        std::cout << "array is not memory contiguous" << std::endl;
    }

    // get the pointer to the data stored in the input matrices
    pic::Complex16* data = (pic::Complex16*)PyArray_DATA(arr);

    // get the dimensions of the array self->C
    int dim_num = PyArray_NDIM( arr );
    npy_intp* dims = PyArray_DIMS(arr);

    // create PIC version of the input matrices   
    if (dim_num == 2) {
        pic::matrix mtx = pic::matrix(data, dims[0], dims[1]);  
        return mtx;
    }
    else if (dim_num == 1) {
        pic::matrix mtx = pic::matrix(data, dims[0], 1);  
        return mtx;
    }
    else {
        std::cout << "numpy2matrix: Wrong matrix dimension was given" << std::endl;
        return pic::matrix(0,0);
    }



} 



/**
@brief Call to create a PicState_int64 representation of a numpy array
*/
pic::PicState_int64 
numpy2PicState_int64(PyObject *arr) {


    // test C-style contiguous memory allocation of the arrays
    if ( !PyArray_IS_C_CONTIGUOUS(arr) ) {
        std::cout << "array is not memory contiguous" << std::endl;
    }

    // get the pointer to the data stored in the input matrices
    int64_t* data = (int64_t*)PyArray_DATA(arr);

    // get the dimensions of the array self->C
    int dim_num = PyArray_NDIM( arr );
    npy_intp* dims = PyArray_DIMS(arr);

    // create PIC version of the input matrices   
    if (dim_num == 1) {
        pic::PicState_int64 state = pic::PicState_int64(data, dims[0]);  
        return state;
    }
    else {
        std::cout << "numpy2PicState_int64: Wrong matrix dimension was given" << std::endl;
        return pic::PicState_int64(0);
    }



} 





extern "C"
{




/**
@brief Method called when a python instance of the class ChinHuhPermanentCalculator_wrapper is destroyed
@param self A pointer pointing to an instance of class ChinHuhPermanentCalculator_wrapper.
*/
static void
ChinHuhPermanentCalculator_wrapper_dealloc(ChinHuhPermanentCalculator_wrapper *self)
{

    // deallocate the instance of class N_Qubit_Decomposition
    release_ChinHuhPermanentCalculator( self->calculator );

    // release numpy arrays
    Py_DECREF(self->matrix);    
    Py_DECREF(self->input_state);    
    Py_DECREF(self->output_state);    

    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
@brief Method called when a python instance of the class ChinHuhPermanentCalculator_wrapper is allocated
@param type A pointer pointing to a structure describing the type of the class ChinHuhPermanentCalculator_wrapper.
*/
static PyObject *
ChinHuhPermanentCalculator_wrapper_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ChinHuhPermanentCalculator_wrapper *self;
    self = (ChinHuhPermanentCalculator_wrapper *) type->tp_alloc(type, 0);
    if (self != NULL) {}

    self->matrix = NULL;
    self->input_state = NULL;
    self->output_state = NULL;

    return (PyObject *) self;
}


/**
@brief Method called when a python instance of the class ChinHuhPermanentCalculator_wrapper is initialized
@param self A pointer pointing to an instance of the class ChinHuhPermanentCalculator_wrapper.
@param args A tuple of the input arguments: qbit_num (integer)
qbit_num: the number of qubits spanning the operations
@param kwds A tuple of keywords
*/
static int
ChinHuhPermanentCalculator_wrapper_init(ChinHuhPermanentCalculator_wrapper *self, PyObject *args, PyObject *kwds)
{
    // The tuple of expected keywords
    static char *kwlist[] = {(char*)"matrix", (char*)"input_state", (char*)"output_state", NULL};

    // initiate variables for input arguments
    PyObject *matrix_arg = NULL;
    PyObject *input_state_arg = NULL;
    PyObject *output_state_arg = NULL;

    // parsing input arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO", kwlist,
                                     &matrix_arg, &input_state_arg, &output_state_arg))
        return -1;

    // convert python object array to numpy C API array
    if ( matrix_arg == NULL ) return -1;
    if ( input_state_arg == NULL ) return -1;
    if ( output_state_arg == NULL ) return -1;

    // establish memory contiguous arrays for C calculations
    if ( PyArray_IS_C_CONTIGUOUS(matrix_arg) ) {
        self->matrix = matrix_arg;
        Py_INCREF(self->matrix); 
    }
    else {
        self->matrix = PyArray_FROM_OTF(matrix_arg, NPY_COMPLEX128, NPY_ARRAY_IN_ARRAY);
    }

    if ( PyArray_IS_C_CONTIGUOUS(input_state_arg) ) {
        self->input_state = input_state_arg;
        Py_INCREF(self->input_state); 
    }
    else {
        self->input_state = PyArray_FROM_OTF(input_state_arg, NPY_INT64, NPY_ARRAY_IN_ARRAY);
    }

    if ( PyArray_IS_C_CONTIGUOUS(output_state_arg) ) {
        self->output_state = output_state_arg;
        Py_INCREF(self->output_state); 
    }
    else {
        self->output_state = PyArray_FROM_OTF(output_state_arg, NPY_INT64, NPY_ARRAY_IN_ARRAY);
    }


    

    // create PIC version of the input matrices
    pic::matrix matrix_mtx = numpy2matrix(self->matrix); 
    pic::PicState_int64 input_state_mtx = numpy2PicState_int64(self->input_state);  
    pic::PicState_int64 output_state_mtx = numpy2PicState_int64(self->output_state);  

    // create instance of class ChinHuhPermanentCalculator
    self->calculator = create_ChinHuhPermanentCalculator( matrix_mtx, input_state_mtx, output_state_mtx );
   
    return 0;
}



/**
@brief Wrapper function to call the calculate method of C++ class CChinHuhPermanentCalculator
@param self A pointer pointing to an instance of the class ChinHuhPermanentCalculator_Wrapper.
@param args A tuple of the input arguments: ??????????????
@param kwds A tuple of keywords
*/
static PyObject *
ChinHuhPermanentCalculator_Wrapper_calculate(ChinHuhPermanentCalculator_wrapper *self)
{

    // start the calculation of the permanent
    pic::Complex16 ret = self->calculator->calculate();

    return Py_BuildValue("D", &ret);
}




/**
@brief Method to call get attribute matrix
*/
static PyObject *
ChinHuhPermanentCalculator_wrapper_getmatrix(ChinHuhPermanentCalculator_wrapper *self, void *closure)
{
    Py_INCREF(self->matrix);
    return self->matrix;
}

/**
@brief Method to call set attribute matrix
*/
static int
ChinHuhPermanentCalculator_wrapper_setmatrix(ChinHuhPermanentCalculator_wrapper *self, PyObject *matrix_arg, void *closure)
{
    // set the array on the Python side
    Py_DECREF(self->matrix);

    // establish memory contiguous arrays for C calculations
    if ( PyArray_IS_C_CONTIGUOUS(matrix_arg) ) {
        self->matrix = matrix_arg;
        Py_INCREF(self->matrix); 
    }
    else {
        self->matrix = PyArray_FROM_OTF(matrix_arg, NPY_COMPLEX128, NPY_ARRAY_IN_ARRAY);
    }


    // create PIC version of the input matrices
    pic::matrix matrix_mtx = numpy2matrix(self->matrix);     

    // update data on the C++ side
    self->calculator->Update_mtx( matrix_mtx );


    return 0;
}





/**
@brief Method to call get the input_state
*/
static PyObject *
ChinHuhPermanentCalculator_wrapper_getinput_state(ChinHuhPermanentCalculator_wrapper *self, void *closure)
{
    Py_INCREF(self->input_state);
    return self->input_state;
}

/**
@brief Method to call set the input_state
*/
static int
ChinHuhPermanentCalculator_wrapper_setinput_state(ChinHuhPermanentCalculator_wrapper *self, PyObject *input_state_arg, void *closure)
{
    // set the array on the Python side
    Py_DECREF(self->input_state); 

    if ( PyArray_IS_C_CONTIGUOUS(input_state_arg) ) {
        self->input_state = input_state_arg;
        Py_INCREF(self->input_state); 
    }
    else {
        self->input_state = PyArray_FROM_OTF(input_state_arg, NPY_INT64, NPY_ARRAY_IN_ARRAY);
    }

    // create PIC version of the input matrices
    pic::PicState_int64 input_state_mtx = numpy2PicState_int64(self->input_state);    

    // update data on the C++ side
    self->calculator->Update_input_state( input_state_mtx );


    return 0;
}




/**
@brief Method to call get the output_state
*/
static PyObject *
ChinHuhPermanentCalculator_wrapper_getoutput_state(ChinHuhPermanentCalculator_wrapper *self, void *closure)
{
    Py_INCREF(self->output_state);
    return self->output_state;
}

/**
@brief Method to call set matrix mean
*/
static int
ChinHuhPermanentCalculator_wrapper_setoutput_state(ChinHuhPermanentCalculator_wrapper *self, PyObject *output_state_arg, void *closure)
{

    // set the array on the Python side
    Py_DECREF(self->output_state); 

    if ( PyArray_IS_C_CONTIGUOUS(output_state_arg) ) {
        self->output_state = output_state_arg;
        Py_INCREF(self->output_state); 
    }
    else {
        self->output_state = PyArray_FROM_OTF(output_state_arg, NPY_INT64, NPY_ARRAY_IN_ARRAY);
    }

    // create PIC version of the input matrices
    pic::PicState_int64 output_state_mtx = numpy2PicState_int64(self->output_state);    

    // update data on the C++ side
    self->calculator->Update_output_state( output_state_mtx );


    return 0;
}



static PyGetSetDef ChinHuhPermanentCalculator_wrapper_getsetters[] = {
    {"matrix", (getter) ChinHuhPermanentCalculator_wrapper_getmatrix, (setter) ChinHuhPermanentCalculator_wrapper_setmatrix,
     "matrix", NULL},
    {"input_state", (getter) ChinHuhPermanentCalculator_wrapper_getinput_state, (setter) ChinHuhPermanentCalculator_wrapper_setinput_state,
     "input_state", NULL},
    {"output_state", (getter) ChinHuhPermanentCalculator_wrapper_getoutput_state, (setter) ChinHuhPermanentCalculator_wrapper_setoutput_state,
     "output_state", NULL},
    {NULL}  /* Sentinel */
};

/**
@brief Structure containing metadata about the members of class ChinHuhPermanentCalculator_wrapper.
*/
static PyMemberDef ChinHuhPermanentCalculator_wrapper_Members[] = {
    {NULL}  /* Sentinel */
};


static PyMethodDef ChinHuhPermanentCalculator_wrapper_Methods[] = {
    {"calculate", (PyCFunction) ChinHuhPermanentCalculator_Wrapper_calculate, METH_NOARGS,
     "Method to calculate the permanent."
    },
    {NULL}  /* Sentinel */
};


/**
@brief A structure describing the type of the class ChinHuhPermanentCalculator_wrapper.
*/
static PyTypeObject ChinHuhPermanentCalculator_wrapper_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "ChinHuhPermanentCalculator_wrapper.ChinHuhPermanentCalculator_wrapper", /*tp_name*/
  sizeof(ChinHuhPermanentCalculator_wrapper), /*tp_basicsize*/
  0, /*tp_itemsize*/
  (destructor) ChinHuhPermanentCalculator_wrapper_dealloc, /*tp_dealloc*/
  #if PY_VERSION_HEX < 0x030800b4
  0, /*tp_print*/
  #endif
  #if PY_VERSION_HEX >= 0x030800b4
  0, /*tp_vectorcall_offset*/
  #endif
  0, /*tp_getattr*/
  0, /*tp_setattr*/
  #if PY_MAJOR_VERSION < 3
  0, /*tp_compare*/
  #endif
  #if PY_MAJOR_VERSION >= 3
  0, /*tp_as_async*/
  #endif
  0, /*tp_repr*/
  0, /*tp_as_number*/
  0, /*tp_as_sequence*/
  0, /*tp_as_mapping*/
  0, /*tp_hash*/
  0, /*tp_call*/
  0, /*tp_str*/
  0, /*tp_getattro*/
  0, /*tp_setattro*/
  0, /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Object to represent a Operation_block class of the QGD package.", /*tp_doc*/
  0, /*tp_traverse*/
  0, /*tp_clear*/
  0, /*tp_richcompare*/
  0, /*tp_weaklistoffset*/
  0, /*tp_iter*/
  0, /*tp_iternext*/
  ChinHuhPermanentCalculator_wrapper_Methods, /*tp_methods*/
  ChinHuhPermanentCalculator_wrapper_Members, /*tp_members*/
  ChinHuhPermanentCalculator_wrapper_getsetters, /*tp_getset*/
  0, /*tp_base*/
  0, /*tp_dict*/
  0, /*tp_descr_get*/
  0, /*tp_descr_set*/
  0, /*tp_dictoffset*/
  (initproc) ChinHuhPermanentCalculator_wrapper_init, /*tp_init*/
  0, /*tp_alloc*/
  ChinHuhPermanentCalculator_wrapper_new, /*tp_new*/
  0, /*tp_free*/
  0, /*tp_is_gc*/
  0, /*tp_bases*/
  0, /*tp_mro*/
  0, /*tp_cache*/
  0, /*tp_subclasses*/
  0, /*tp_weaklist*/
  0, /*tp_del*/
  0, /*tp_version_tag*/
  #if PY_VERSION_HEX >= 0x030400a1
  0, /*tp_finalize*/
  #endif
  #if PY_VERSION_HEX >= 0x030800b1
  0, /*tp_vectorcall*/
  #endif
  #if PY_VERSION_HEX >= 0x030800b4 && PY_VERSION_HEX < 0x03090000
  0, /*tp_print*/
  #endif
};

/**
@brief Structure containing metadata about the module.
*/
static PyModuleDef ChinHuhPermanentCalculator_wrapper_Module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "ChinHuhPermanentCalculator_wrapper",
    .m_doc = "Python binding for class ChinHuhPermanentCalculator",
    .m_size = -1,
};

/**
@brief Method called when the Python module is initialized
*/
PyMODINIT_FUNC
PyInit_Boson_Sampling_Utilities_wrapper(void)
{
    // initialize Numpy API
    import_array();

    PyObject *m;
    if (PyType_Ready(&ChinHuhPermanentCalculator_wrapper_Type) < 0)
        return NULL;

    m = PyModule_Create(&ChinHuhPermanentCalculator_wrapper_Module);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ChinHuhPermanentCalculator_wrapper_Type);
    if (PyModule_AddObject(m, "ChinHuhPermanentCalculator_wrapper", (PyObject *) &ChinHuhPermanentCalculator_wrapper_Type) < 0) {
        Py_DECREF(&ChinHuhPermanentCalculator_wrapper_Type);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}



}
