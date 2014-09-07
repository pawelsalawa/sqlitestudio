#ifndef UIDEBUG_H
#define UIDEBUG_H

#include "guiSQLiteStudio_global.h"
#include <QtDebug>

GUI_API_EXPORT void uiMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
GUI_API_EXPORT void setUiDebug(bool enabled, bool useUiConsole = true);
GUI_API_EXPORT void showUiDebugConsole();
GUI_API_EXPORT bool isDebugEnabled();
GUI_API_EXPORT bool isDebugConsoleEnabled();

#endif // UIDEBUG_H
