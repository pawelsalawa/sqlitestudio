#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include "services/exportmanager.h"
#include "db/queryexecutor.h"
#include <QObject>
#include <QRunnable>
#include <QMutex>

class Db;

class ExportWorker : public QObject, public QRunnable
{
        Q_OBJECT
    public:
        explicit ExportWorker(ExportPlugin* plugin, ExportManager::StandardExportConfig* config, QIODevice* output, QObject *parent = 0);
        ~ExportWorker();

        void run();
        void prepareExportQueryResults(Db* db, const QString& query);
        void prepareExportDatabase(Db* db, const QStringList& objectListToExport);
        void prepareExportTable(Db* db, const QString& database, const QString& table);
        void interrupt();

    private:
        bool exportQueryResults();
        bool exportDatabase();
        bool exportTable();
        bool exportTableInternal(const QString& database, const QString& table, const QString& ddl, SqlQueryPtr results, bool databaseExport);
        QList<ExportManager::ExportObjectPtr> collectDbObjects(QString* errorMessage);
        void queryTableDataToExport(Db* db, const QString& table, SqlQueryPtr& dataPtr, QString* errorMessage) const;
        bool isInterrupted();

        ExportPlugin* plugin = nullptr;
        ExportManager::StandardExportConfig* config = nullptr;
        QIODevice* output = nullptr;
        ExportManager::ExportMode exportMode = ExportManager::UNDEFINED;
        Db* db = nullptr;
        QString query;
        QString database;
        QString table;
        QStringList objectListToExport;
        QueryExecutor* executor;
        bool interrupted = false;
        QMutex interruptMutex;

    signals:
        void finished(bool result, QIODevice* output);
};

#endif // EXPORTWORKER_H
