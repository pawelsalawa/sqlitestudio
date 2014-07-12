#ifndef UIAPIEXPORT_H
#define UIAPIEXPORT_H

#include <QtCore/qglobal.h>

#if defined(SQLITESTUDIO_BINARY)
#  define UI_API_EXPORT Q_DECL_EXPORT
#else
#  define UI_API_EXPORT Q_DECL_IMPORT
#endif

#endif // UIAPIEXPORT_H
