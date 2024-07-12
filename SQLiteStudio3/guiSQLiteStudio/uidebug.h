#ifndef UIDEBUG_H
#define UIDEBUG_H

#include "guiSQLiteStudio_global.h"
#include <QtDebug>
#include <QObject>
#include <QTextStream>

class QFile;

class GUI_API_EXPORT MsgHandlerThreadProxy : public QObject
{
        Q_OBJECT

    public:
        explicit MsgHandlerThreadProxy(QObject* parent = 0);
        MsgHandlerThreadProxy(const QString& file, QObject* parent = 0);
        ~MsgHandlerThreadProxy();

    private:
        void init();
        void initFile(const QString& fileName);

        static QStringList ignoredWarnings;

        QFile* outFile = nullptr;
        QTextStream outFileStream;

    public slots:
        void debug(const QString& msg);
        void warn(const QString& msg);
        void critical(const QString& msg);
        void fatal(const QString& msg);

    signals:
        void debugRequested(const QString& msg);
        void warnRequested(const QString& msg);
        void criticalRequested(const QString& msg);
        void fatalRequested(const QString& msg);

    private slots:
        void print(const QString& txt);
        void printToFile(const QString& txt);
};

GUI_API_EXPORT void uiMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
GUI_API_EXPORT void setUiDebug(bool enabled, bool useUiConsole = true, const QString& file = QString());
GUI_API_EXPORT void showUiDebugConsole();
GUI_API_EXPORT bool isDebugEnabled();
GUI_API_EXPORT bool isDebugConsoleEnabled();

#endif // UIDEBUG_H
