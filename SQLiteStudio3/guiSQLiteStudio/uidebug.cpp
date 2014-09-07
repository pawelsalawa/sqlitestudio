#include "uidebug.h"
#include "common/unused.h"
#include "qio.h"
#include "debugconsole.h"
#include "common/global.h"
#include <QTime>

DebugConsole* sqliteStudioUiDebugConsole = nullptr;
bool UI_DEBUG_ENABLED = false;
bool UI_DEBUG_CONSOLE = true;

void uiMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!UI_DEBUG_ENABLED)
        return;

    UNUSED(context);

    static const QString dbgMsg = QStringLiteral("[%1] DEBUG:    %2");
    static const QString wrnMsg = QStringLiteral("[%1] WARNING:  %2");
    static const QString criMsg = QStringLiteral("[%1] CRITICAL: %2");
    static const QString fatMsg = QStringLiteral("[%1] FATAL:    %2");

    QString time = QTime::currentTime().toString("HH:mm:ss.zzz");
    if (UI_DEBUG_CONSOLE)
    {
        switch (type) {
            case QtDebugMsg:
                sqliteStudioUiDebugConsole->debug(dbgMsg.arg(time, msg));
                break;
            case QtWarningMsg:
                sqliteStudioUiDebugConsole->warning(wrnMsg.arg(time, msg));
                break;
            case QtCriticalMsg:
                sqliteStudioUiDebugConsole->critical(criMsg.arg(time, msg));
                break;
            case QtFatalMsg:
                sqliteStudioUiDebugConsole->fatal(fatMsg.arg(time, msg));
                abort();
        }
    }
    else
    {
        QString txt;
        switch (type) {
            case QtDebugMsg:
                txt = dbgMsg;
                break;
            case QtWarningMsg:
                txt = wrnMsg;
                break;
            case QtCriticalMsg:
                txt = criMsg;
                break;
            case QtFatalMsg:
                txt = fatMsg;
                abort();
        }

        qOut << txt.arg(time, msg) << "\n";
        qOut.flush();
    }
}

void setUiDebug(bool enabled, bool useUiConsole)
{
    UI_DEBUG_ENABLED = enabled;
    UI_DEBUG_CONSOLE =  useUiConsole;
    safe_delete(sqliteStudioUiDebugConsole);
    if (enabled && useUiConsole)
        sqliteStudioUiDebugConsole = new DebugConsole();
}

void showUiDebugConsole()
{
    if (sqliteStudioUiDebugConsole)
        sqliteStudioUiDebugConsole->show();
}

bool isDebugEnabled()
{
    return UI_DEBUG_ENABLED;
}

bool isDebugConsoleEnabled()
{
    return UI_DEBUG_CONSOLE;
}
