#include "exportworker.h"
#include "plugins/exportplugin.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include <QDebug>

ExportWorker::ExportWorker(ExportPlugin* plugin, ExportManager::StandardExportConfig* config, QIODevice* output, QObject *parent) :
    QObject(parent), plugin(plugin), config(config), output(output)
{
    executor = new QueryExecutor();
    executor->setAsyncMode(false); // worker runs in its own thread, we don't need (and we don't want) async executor
}

ExportWorker::~ExportWorker()
{
    safe_delete(executor);
}

void ExportWorker::run()
{
    bool res = false;
    switch (exportMode)
    {
        case ExportManager::QUERY_RESULTS:
        {
            res = exportQueryResults();
            break;
        }
        case ExportManager::DATABASE:
        {
            res = exportDatabase();
            break;
        }
        case ExportManager::TABLE:
        {
            res = exportTable();
            break;
        }
        case ExportManager::UNDEFINED:
            qCritical() << "Started ExportWorker with UNDEFINED mode.";
            res = false;
            break;
    }

    emit finished(res, output);
}

void ExportWorker::prepareExportQueryResults(Db* db, const QString& query)
{
    this->db = db;
    this->query = query;
    exportMode = ExportManager::QUERY_RESULTS;
}

void ExportWorker::prepareExportDatabase(Db* db, const QStringList& objectListToExport)
{
    this->db = db;
    this->objectListToExport = objectListToExport;
    exportMode = ExportManager::DATABASE;
}

void ExportWorker::prepareExportTable(Db* db, const QString& database, const QString& table)
{
    this->db = db;
    this->database = database;
    this->table = table;
    exportMode = ExportManager::TABLE;
}

bool ExportWorker::exportQueryResults()
{
    executor->setDb(db);
    executor->exec(query);
    SqlResultsPtr results = executor->getResults();
    if (!results)
    {
        qCritical() << "Null results from executor in ExportWorker.";
        return false;
    }

    if (results->isError())
    {
        notifyError(tr("Error while exporting query results: %1").arg(results->getErrorText()));
        return false;
    }

    QList<QueryExecutor::ResultColumnPtr> resultColumns = executor->getResultColumns();
    return plugin->exportQueryResults(db, query, results, resultColumns, output, *config);
}

bool ExportWorker::exportDatabase()
{
    QString err;
    QList<ExportManager::ExportObjectPtr> dbObjects = collectDbObjects(&err);
    if (!err.isNull())
    {
        notifyError(err);
        return false;
    }

    return plugin->exportDatabase(db, dbObjects, output, *config);
}

bool ExportWorker::exportTable()
{
    static const QString sql = QStringLiteral("SELECT * FROM %1");

    SqlResultsPtr results;

    if (config->exportData)
    {
        results = db->exec(sql.arg(table));
        if (results->isError())
        {
            notifyError(tr("Error while exporting table data: %1").arg(results->getErrorText()));
            return false;
        }
    }

    SchemaResolver resolver(db);
    QString ddl = resolver.getObjectDdl(database, table);

    return plugin->exportTable(db, database, table, ddl, results, output, *config);
}

QList<ExportManager::ExportObjectPtr> ExportWorker::collectDbObjects(QString* errorMessage)
{
    SchemaResolver resolver(db);
    QStringList tables = resolver.getTables();
    QStringList indexes = resolver.getIndexes();
    QStringList triggers = resolver.getTriggers();
    QStringList views = resolver.getViews();

    QList<ExportManager::ExportObjectPtr> objectsToExport;
    ExportManager::ExportObjectPtr exportObj;
    for (const QString& obj : objectListToExport)
    {
        exportObj = ExportManager::ExportObjectPtr::create();
        if (tables.contains(obj))
        {
            objectsToExport << getTableObjToExport(db, obj, errorMessage);
            if (!errorMessage->isNull())
                return objectsToExport;
        }
        else if (indexes.contains(obj))
        {
            exportObj->type = ExportManager::ExportObject::INDEX;
            exportObj->name = obj;
            objectsToExport << exportObj;
        }
        else if (triggers.contains(obj))
        {
            exportObj->type = ExportManager::ExportObject::TRIGGER;
            exportObj->name = obj;
            objectsToExport << exportObj;
        }
        else if (views.contains(obj))
        {
            exportObj->type = ExportManager::ExportObject::VIEW;
            exportObj->name = obj;
            objectsToExport << exportObj;
        }
    }

    return objectsToExport;
}

ExportManager::ExportObjectPtr ExportWorker::getTableObjToExport(Db* db, const QString& table, QString* errorMessage) const
{
    static const QString sql = QStringLiteral("SELECT * FROM %1");

    ExportManager::ExportObjectPtr exportObject = ExportManager::ExportObjectPtr::create();
    exportObject->type = ExportManager::ExportObject::TABLE;
    exportObject->name = table;

    if (config->exportData)
    {
        exportObject->data = db->exec(sql.arg(table));
        if (exportObject->data->isError())
        {
            *errorMessage = tr("Error while reading data to export from table %1: %2").arg(table).arg(exportObject->data->getErrorText());
            return exportObject;
        }
    }

    return exportObject;
}

