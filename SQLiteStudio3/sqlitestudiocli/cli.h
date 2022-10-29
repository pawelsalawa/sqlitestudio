#ifndef CLI_H
#define CLI_H

#include "db/db.h"
#include <QObject>
#include <QTextStream>
#include <QStringList>
#include <QTime>

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
        static void dispose();

        void start();
        void setCurrentDb(Db* db);
        Db* getCurrentDb() const;
        void exit();
        QStringList getHistory() const;
        QString getLine() const;
        void applyHistoryLimit();

    private:
        explicit CLI(QObject* parent = nullptr);

        void waitForExecution();
        bool isComplete(const QString& contents) const;
        void loadHistory();
        void addHistory(const QString& text);
        void println(const QString& msg = QString());
        int historyLength() const;

        static CLI* instance;

        QString lastHistoryEntry;
        QThread* thread = nullptr;
        Db* currentDb = nullptr;
        bool executionFinished = false;
        bool doExit = false;
        QString line;

    private slots:
        void printInfo(const QString& msg);
        void printWarn(const QString& msg);
        void printError(const QString& msg);

    public slots:
        void doWork();
        void done();
        void executionComplete();
        void clearHistory();
        bool openDbFile(const QString& path);

    signals:
        void execCommand(CliCommand* cmd);
};

#endif // CLI_H
