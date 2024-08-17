#ifndef STATICPYTHONAPI_H
#define STATICPYTHONAPI_H

#include <Python.h>
#include <frameobject.h>

#ifndef METH_FASTCALL
// In Python pre-3.6 SDK, this is undefined
#define METH_FASTCALL 0x0080
#endif

// A couple of compatibility defines for Python SDK before 3.11.
#if PY_VERSION_HEX < 0x030b0000
#define PyFrame_GetGlobals(fr) ((fr)->f_globals)
#define PyFrame_GetLocals(fr) ((fr)->f_locals)
#endif

#endif  // STATICPYTHONAPI_H
