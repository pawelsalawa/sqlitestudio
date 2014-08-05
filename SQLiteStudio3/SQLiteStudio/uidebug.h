#ifndef UIDEBUG_H
#define UIDEBUG_H

#include <QtDebug>

void uiMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void setUiDebug(bool enabled, bool useUiConsole = true);
void showUiDebugConsole();
bool isDebugEnabled();
bool isDebugConsoleEnabled();

#endif // UIDEBUG_H
