#ifndef SQLFILEEXECUTOR_H
#define SQLFILEEXECUTOR_H

#include "coreSQLiteStudio_global.h"
#include <QObject>
#include <QTextStream>

class Db;

class API_EXPORT SqlFileExecutor : public QObject
{
    Q_OBJECT

    public:
        enum ExecutionMode
        {
            STRICT_MODE, // not just STRICT, cause it's reserved symbol in qwindefs_win.h
            PERMISSIVE,
            EXTENDED
        };

        explicit SqlFileExecutor(QObject *parent = nullptr);
        void execSqlFromFile(Db* db, const QString& filePath, bool ignoreErrors, QString codec, bool async = true);
        bool isExecuting() const;
        ExecutionMode getExecutionMode() const;
        void setExecutionMode(ExecutionMode newExecutionMode);

    private:
        bool execQueryFromFile(Db* db, const QString& sql);
        void execInThread();
        void handleExecutionResults(Db* db, int executed, int attemptedExecutions, bool ok, bool ignoreErrors, int millis);
        QList<QPair<QString, QString>> executeFromStream(QTextStream& stream, int& executed, int& attemptedExecutions, bool& ok, qint64 fileSize);
        bool shouldSkipQuery(const QString& sql, bool isEnd) const;
        QString processDotCommands(const QString& sql, QList<QPair<QString, QString> >& errors);
        void handleDotCommand(const QString& cmdLine, QList<QPair<QString, QString> >& errors);
        QStringList splitArgs(const QString& line);
        void rollback();
        bool commit();

        QAtomicInt executionInProgress = 0;
        Db* db = nullptr;
        bool fkWasEnabled = true;
        bool ignoreErrors = false;
        QString codec;
        QString filePath;
        QString txName;
        ExecutionMode executionMode = STRICT_MODE;

    public slots:
        void stopExecution();

    signals:
        void schemaNeedsRefreshing(Db* db);
        void updateProgress(int value);
        void execEnded();
        void execErrors(const QList<QPair<QString, QString>>& errors, bool rolledBack);
};

#endif // SQLFILEEXECUTOR_H
