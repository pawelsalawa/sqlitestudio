#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include "services/exportmanager.h"
#include "db/queryexecutor.h"
#include <QObject>
#include <QRunnable>

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

    private:
        bool exportQueryResults();
        bool exportDatabase();
        bool exportTable();
        QList<ExportManager::ExportObjectPtr> collectDbObjects(QString* errorMessage);
        ExportManager::ExportObjectPtr getTableObjToExport(Db* db, const QString& table, QString* errorMessage) const;

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

    signals:
        void finished(bool result, QIODevice* output);
};

#endif // EXPORTWORKER_H
