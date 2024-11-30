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
        explicit SqlFileExecutor(QObject *parent = nullptr);
        void execSqlFromFile(Db* db, const QString& filePath, bool ignoreErrors, QString codec, bool async = true);
        bool isExecuting() const;

    private:
        bool execQueryFromFile(Db* db, const QString& sql);
        void execInThread();
        void handleExecutionResults(Db* db, int executed, int attemptedExecutions, bool ok, bool ignoreErrors, int millis);
        QList<QPair<QString, QString>> executeFromStream(QTextStream& stream, int& executed, int& attemptedExecutions, bool& ok, qint64 fileSize);
        bool shouldSkipQuery(const QString& sql);

        QAtomicInt executionInProgress = 0;
        Db* db = nullptr;
        //bool fkWasEnabled = true; // See comment about fkWasEnabled in cpp file.
        bool ignoreErrors = false;
        QString codec;
        QString filePath;

    public slots:
        void stopExecution();

    signals:
        void schemaNeedsRefreshing(Db* db);
        void updateProgress(int value);
        void execEnded();
        void execErrors(const QList<QPair<QString, QString>>& errors, bool rolledBack);
};

#endif // SQLFILEEXECUTOR_H
