#ifndef CLI_H
#define CLI_H

#include "db/db.h"
#include <QObject>
#include <QTextStream>
#include <QStringList>

class QThread;
class QFile;
class DbManager;
class CliCommand;

class CLI : public QObject
{
    Q_OBJECT

    public:
        ~CLI();

        static CLI* getInstance();

        void start();
        void setCurrentDb(Db* db);
        Db* getCurrentDb() const;
        void exit();
        QStringList getHistory() const;
        QString getLine() const;

    private:
        CLI();

        void waitForExecution();
        bool isComplete(const QString& contents) const;
        void loadHistory();
        void saveHistory();
        void println(const QString& msg = QString());

        static CLI* instance;

        QThread* thread;
        Db* currentDb = nullptr;
        bool executionFinished = false;
        bool doExit = false;
        QString line;

    signals:
        void execCommand(CliCommand* cmd);

    public slots:
        void doWork();
        void done();
        void executionComplete();
        void clearHistory();
        void openDbFile(const QString& path);
};

#endif // CLI_H
