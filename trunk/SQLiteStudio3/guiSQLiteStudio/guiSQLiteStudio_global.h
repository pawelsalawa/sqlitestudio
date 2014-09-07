#ifndef GUISQLITESTUDIO_GLOBAL_H
#define GUISQLITESTUDIO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GUISQLITESTUDIO_LIBRARY)
#  define GUI_API_EXPORT Q_DECL_EXPORT
#else
#  define GUI_API_EXPORT Q_DECL_IMPORT
#endif

#endif // GUISQLITESTUDIO_GLOBAL_H
