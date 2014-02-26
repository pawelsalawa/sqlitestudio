#ifndef CLI_H
#define CLI_H

#include "db/db.h"
#include <QObject>
#include <QTextStream>
#include <QStringList>
#include <QWaitCondition>
#include <QMutex>

class QThread;
class QFile;
class DbManager;
class CliCommand;

class CLI : public QObject
{
    Q_OBJECT

    public:
        explicit CLI(QObject *parent = 0);
        ~CLI();

        void start();
        void setCurrentDb(Db* db);
        Db* getCurrentDb();
        quint32 getMaxColLength();
        void setMaxColLength(quint32 value);
        void exit();

    private:
        void println(const QString& msg);
        void waitForExecution();

        DbManager* dbManager;
        QThread* thread;
        Db* currentDb = nullptr;
        quint32 maxResultColumnLength = 20;
        QMutex waitMutex;
        QWaitCondition waitForExec;
        bool executionFinished = false;
        bool doExit = false;
        QString prompt;

    signals:
        void execCommand(CliCommand* cmd, QStringList args);

    public slots:
        void doWork();
        void done();
        void executionComplete();
};

#endif // CLI_H
