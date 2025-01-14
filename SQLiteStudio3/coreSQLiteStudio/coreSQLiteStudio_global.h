#ifndef CORESQLITESTUDIO_GLOBAL_H
#define CORESQLITESTUDIO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CORESQLITESTUDIO_LIBRARY)
#  define API_EXPORT Q_DECL_EXPORT
#else
#  define API_EXPORT
//#  define API_EXPORT Q_DECL_IMPORT
#endif

#ifdef Q_OS_WIN
#   define PATH_LIST_SEPARATOR ";"
#else
#   define PATH_LIST_SEPARATOR ":"
#endif

#endif // CORESQLITESTUDIO_GLOBAL_H
