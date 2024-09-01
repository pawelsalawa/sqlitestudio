#ifndef DYNAMICPYTHONAPI_H
#define DYNAMICPYTHONAPI_H

// We require Python 3.x headers
#include <Python.h>
#include <frameobject.h>

#include <QLibrary>

#ifndef METH_FASTCALL
// In Python pre-3.6 SDK, this is undefined
#define METH_FASTCALL 0x0080
#endif

// In Python 3.x SDK, we need these prototypes for binding to 2.7 library
extern PyObject *Py_InitModule4(const char*, PyMethodDef*, const char*, PyObject*, int);
extern PyObject *_PyUnicode_AsDefaultEncodedString(PyObject*, const char*);
extern int PyUnicode_SetDefaultEncoding(const char*);

#if PY_VERSION_HEX < 0x030b0000
// In Python pre-3.11 SDK, we need these prototypes from future
extern PyObject* PyFrame_GetGlobals(PyFrameObject*);
extern PyObject* PyFrame_GetLocals(PyFrameObject*);
#endif

// Define a list of symbols, which is then used once in class declaration and
// three times in the implementation
#define DYNAMICPYTHONAPI_MEMBERS \
    PYAPI_DATA(PyBool_Type) \
    PYAPI_DATA(PyByteArray_Type) \
    PYAPI_DATA(PyFloat_Type) \
    PYAPI_DATA(PySet_Type) \
    PYAPI_DATA(PyExc_RuntimeError) \
    PYAPI_METHOD(Py_DecRef) \
    PYAPI_METHOD(Py_EndInterpreter) \
    PYAPI_METHOD(Py_Finalize) \
    PYAPI_METHOD(Py_GetVersion) \
    PYAPI_METHOD(Py_IncRef) \
    PYAPI_METHOD(Py_Initialize) \
    PYAPI_METHOD(Py_NewInterpreter) \
    PYAPI_METHOD(PyBool_FromLong) \
    PYAPI_METHOD(PyByteArray_AsString) \
    PYAPI_METHOD(PyByteArray_Size) \
    PYAPI_METHOD(PyBytes_AsStringAndSize) \
    PYAPI_METHOD(PyBytes_FromStringAndSize) \
    PYAPI_METHOD(PyDict_Clear) \
    PYAPI_METHOD(PyDict_GetItemString) \
    PYAPI_METHOD(PyDict_New) \
    PYAPI_METHOD(PyDict_Next) \
    PYAPI_METHOD(PyDict_SetItemString) \
    PYAPI_METHOD(PyErr_Clear) \
    PYAPI_METHOD(PyErr_Fetch) \
    PYAPI_METHOD(PyErr_Occurred) \
    PYAPI_METHOD(PyErr_SetString) \
    PYAPI_METHOD(PyEval_GetFrame) \
    PYAPI_METHOD(PyEval_InitThreads) \
    PYAPI_METHOD(PyEval_ReleaseThread) \
    PYAPI_METHOD(PyEval_RestoreThread) \
    PYAPI_METHOD(PyFloat_AsDouble) \
    PYAPI_METHOD(PyFloat_FromDouble) \
    PYAPI_METHOD(PyFrame_GetGlobals) \
    PYAPI_METHOD(PyFrame_GetLocals) \
    PYAPI_METHOD(PyFrame_FastToLocals) \
    PYAPI_METHOD(PyImport_AddModule) \
    PYAPI_METHOD(PyImport_AppendInittab) \
    PYAPI_METHOD(PyIter_Next) \
    PYAPI_METHOD(PyList_Append) \
    PYAPI_METHOD(PyList_GetItem) \
    PYAPI_METHOD(PyList_New) \
    PYAPI_METHOD(PyList_SetItem) \
    PYAPI_METHOD(PyList_Size) \
    PYAPI_METHOD(PyLong_AsLongLong) \
    PYAPI_METHOD(PyLong_FromLongLong) \
    PYAPI_METHOD(PyLong_FromUnsignedLongLong) \
    PYAPI_METHOD(PyMapping_Check) \
    PYAPI_METHOD(PyMapping_GetItemString) \
    PYAPI_METHOD(PyModule_Create2) \
    PYAPI_METHOD(PyModule_GetDict) \
    PYAPI_METHOD(PyObject_CallObject) \
    PYAPI_METHOD(PyObject_GetAttrString) \
    PYAPI_METHOD(PyObject_GetIter) \
    PYAPI_METHOD(PyObject_IsTrue) \
    PYAPI_METHOD(PyObject_Repr) \
    PYAPI_METHOD(PyRun_SimpleStringFlags) \
    PYAPI_METHOD(PyRun_StringFlags) \
    PYAPI_METHOD(PyThreadState_Get) \
    PYAPI_METHOD(PyTuple_GetItem) \
    PYAPI_METHOD(PyTuple_New) \
    PYAPI_METHOD(PyTuple_SetItem) \
    PYAPI_METHOD(PyTuple_Size) \
    PYAPI_METHOD(PyType_IsSubtype) \
    PYAPI_METHOD(PyUnicode_AsUTF8AndSize) \
    PYAPI_METHOD(PyUnicode_AsUTF8String) \
    PYAPI_METHOD(PyUnicode_FromStringAndSize)

class DynamicPythonApi
{
    public:
        static bool bindSymbols(QLibrary &library);
        static PyObject* py27compat_PyModule_Create2(PyModuleDef* module, int apiver);
        static const char* py27compat_PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *size);

#define PYAPI_DATA(name) static typeof(::name)* p##name;
#define PYAPI_METHOD(name) static typeof(::name)* name;
// We need PyEval_InitThreads deprecated since Python 3.7 SDK
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        DYNAMICPYTHONAPI_MEMBERS
#pragma GCC diagnostic pop
        // These are all Python 2.7-only symbols we need:
        PYAPI_METHOD(_PyUnicode_AsDefaultEncodedString)
        PYAPI_METHOD(Py_InitModule4)
        PYAPI_METHOD(PyUnicode_SetDefaultEncoding)
#undef PYAPI_DATA
#undef PYAPI_METHOD
};

// Redefine Python macros

#undef Py_DECREF
#undef Py_INCREF
#undef Py_XDECREF
#undef PyBool_Check
#undef PyByteArray_Check
#undef PyBytes_Check
#undef PyDict_Check
#undef PyFloat_Check
#undef PyObject_TypeCheck
#undef PyList_Check
#undef PyList_GET_SIZE
#undef PyLong_Check
#undef PySet_Check
#undef PyTuple_Check
#undef PyUnicode_Check

#define Py_DECREF Py_DecRef
#define Py_INCREF Py_IncRef
#define Py_XDECREF Py_DecRef

#define PyObject_TypeCheck(o, t) (Py_IS_TYPE((o), (t)) || PyType_IsSubtype(Py_TYPE((o)), (t)))
#define PyBool_Check(x)          (Py_TYPE((x)) == pPyBool_Type)
#define PyByteArray_Check(x)     PyObject_TypeCheck((x), pPyByteArray_Type)
#define PyBytes_Check(x)         PyType_HasFeature(Py_TYPE((x)), Py_TPFLAGS_BYTES_SUBCLASS)
#define PyDict_Check(x)	         PyType_HasFeature(Py_TYPE((x)), Py_TPFLAGS_DICT_SUBCLASS)
#define PyFloat_Check(x)         PyObject_TypeCheck((x), pPyFloat_Type)
#define PyList_Check(x)          PyType_HasFeature(Py_TYPE((x)), Py_TPFLAGS_LIST_SUBCLASS)
#define PyList_GET_SIZE(x)       (_PyVarObject_CAST((x))->ob_size)
#define PyLong_Check(x)          PyType_HasFeature(Py_TYPE((x)), Py_TPFLAGS_LONG_SUBCLASS)
#define PySet_Check(x)           PyObject_TypeCheck((x), pPySet_Type)
#define PyTuple_Check(x)         PyType_HasFeature(Py_TYPE((x)), Py_TPFLAGS_TUPLE_SUBCLASS)
#define PyUnicode_Check(x)       PyType_HasFeature(Py_TYPE((x)), Py_TPFLAGS_UNICODE_SUBCLASS)

// There are a few constructs for which placing the stuff in the class scope is not enough
#ifndef DYNAMICPYTHONAPI_INTERNAL
#define PyExc_RuntimeError (*pPyExc_RuntimeError)
#undef PyModule_Create
#define PyModule_Create(m) DynamicPythonApi::PyModule_Create2((m), PYTHON_API_VERSION)
#endif

#endif  // DYNAMICPYTHONAPI_H
