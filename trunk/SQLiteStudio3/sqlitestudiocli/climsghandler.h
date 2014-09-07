#ifndef CLIMSGHANDLER_H
#define CLIMSGHANDLER_H

#include <QtDebug>

void cliMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void setCliDebug(bool enabled);

#endif // CLIMSGHANDLER_H
