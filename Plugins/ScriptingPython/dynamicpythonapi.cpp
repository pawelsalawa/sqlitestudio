#define DYNAMICPYTHONAPI_INTERNAL
#include "dynamicpythonapi.h"
#include <QDebug>

// Needed to build a Python 2.7..3.10 ABI compatible plugin with Python 3.11+ SDK
struct py_compat_frame_head {
    PyObject_VAR_HEAD
    struct _frame *f_back;
    PyCodeObject *f_code;
    PyObject *f_builtins;
    PyObject *f_globals;
    PyObject *f_locals;
};

// Define all variables
#define PYAPI_DATA(x)   typeof(DynamicPythonApi::p##x) DynamicPythonApi::p##x = nullptr;
#define PYAPI_METHOD(x) typeof(DynamicPythonApi::x) DynamicPythonApi::x = nullptr;
DYNAMICPYTHONAPI_MEMBERS
PYAPI_METHOD(_PyUnicode_AsDefaultEncodedString)
PYAPI_METHOD(Py_InitModule4)
PYAPI_METHOD(PyUnicode_SetDefaultEncoding)
#undef PYAPI_DATA
#undef PYAPI_METHOD

PyObject* DynamicPythonApi::py27compat_PyModule_Create2(PyModuleDef* module, int apiver)
{
    return Py_InitModule4(module->m_name, module->m_methods, module->m_doc, NULL, apiver);
}

const char* DynamicPythonApi::py27compat_PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *size)
{
    char *buf;
    PyObject *bytes = _PyUnicode_AsDefaultEncodedString(unicode, "strict");
    if (PyBytes_AsStringAndSize(bytes, &buf, size))
        return NULL;
    return const_cast<const char *>(buf);
}


bool DynamicPythonApi::bindSymbols(QLibrary &library)
{
    // First, try to bind all DYNAMICPYTHONAPI_MEMBERS
#define PYAPI_DATA(x)   p##x = (typeof(p##x))library.resolve(#x);
#define PYAPI_METHOD(x) x = (typeof(x))library.resolve(#x);
    DYNAMICPYTHONAPI_MEMBERS

    // Next, try resolve replacements for missing symbols
#define PYAPI_METHOD_DEFAULT(x, y) \
    if (x == nullptr) \
        x = (typeof(::x)*)y

    // Python 2.7
    PYAPI_METHOD_DEFAULT(PyBytes_AsStringAndSize, library.resolve("PyString_AsStringAndSize"));
    PYAPI_METHOD_DEFAULT(PyBytes_FromStringAndSize, library.resolve("PyString_FromStringAndSize"));
    if (PyModule_Create2 == nullptr)
    {
        PYAPI_METHOD(Py_InitModule4)
        PYAPI_METHOD_DEFAULT(Py_InitModule4, library.resolve("Py_InitModule4_64"));
        if (Py_InitModule4 != nullptr)
            PYAPI_METHOD_DEFAULT(PyModule_Create2, py27compat_PyModule_Create2);
    }
    PYAPI_METHOD_DEFAULT(PyUnicode_AsUTF8String, library.resolve("PyUnicodeUCS4_AsUTF8String"));
    PYAPI_METHOD_DEFAULT(PyUnicode_AsUTF8String, library.resolve("PyUnicodeUCS2_AsUTF8String"));
    if (PyUnicode_AsUTF8AndSize == nullptr)
    {
        _PyUnicode_AsDefaultEncodedString = nullptr;
        PYAPI_METHOD_DEFAULT(_PyUnicode_AsDefaultEncodedString, library.resolve("_PyUnicodeUCS4_AsDefaultEncodedString"));
        PYAPI_METHOD_DEFAULT(_PyUnicode_AsDefaultEncodedString, library.resolve("_PyUnicodeUCS2_AsDefaultEncodedString"));
        PyUnicode_SetDefaultEncoding = nullptr;
        PYAPI_METHOD_DEFAULT(PyUnicode_SetDefaultEncoding, library.resolve("PyUnicodeUCS4_SetDefaultEncoding"));
        PYAPI_METHOD_DEFAULT(PyUnicode_SetDefaultEncoding, library.resolve("PyUnicodeUCS2_SetDefaultEncoding"));
        if (PyUnicode_SetDefaultEncoding != nullptr &&
            _PyUnicode_AsDefaultEncodedString != nullptr)
            PYAPI_METHOD_DEFAULT(PyUnicode_AsUTF8AndSize, py27compat_PyUnicode_AsUTF8AndSize);
    }
    PYAPI_METHOD_DEFAULT(PyUnicode_FromStringAndSize, library.resolve("PyUnicodeUCS4_FromStringAndSize"));
    PYAPI_METHOD_DEFAULT(PyUnicode_FromStringAndSize, library.resolve("PyUnicodeUCS2_FromStringAndSize"));

    // Python 2.7..3.10
    PYAPI_METHOD_DEFAULT(PyFrame_GetGlobals, [](PyFrameObject *frame) {
        return reinterpret_cast<struct py_compat_frame_head*>(frame)->f_globals;
    });
    PYAPI_METHOD_DEFAULT(PyFrame_GetLocals, [](PyFrameObject *frame) {
        return reinterpret_cast<struct py_compat_frame_head*>(frame)->f_locals;
    });

#undef PYAPI_DATA
#undef PYAPI_METHOD
#undef PYAPI_METHOD_DEFAULT

    // Now, report any required DYNAMICPYTHONAPI_MEMBERS that could not be bound.
    bool result = true;
#define PYAPI_DATA(x)   if (p##x == nullptr) { qWarning() << "Could not bind symbol: "#x; result = false; }
#define PYAPI_METHOD(x) if (x == nullptr) { qWarning() << "Could not bind symbol: "#x; result = false; }
    DYNAMICPYTHONAPI_MEMBERS
#undef PYAPI_DATA
#undef PYAPI_METHOD
    return result;
}
