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
