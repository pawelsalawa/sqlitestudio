#include "climsghandler.h"
#include "qio.h"
#include "cli_config.h"
#include "common/unused.h"

bool cliDebug = false;

void cliMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!cliDebug)
        return;

    UNUSED(context);

    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            abort();
    }

    qOut << txt << "\n";
    qOut.flush();
}

void setCliDebug(bool enabled)
{
    cliDebug = enabled;
}
