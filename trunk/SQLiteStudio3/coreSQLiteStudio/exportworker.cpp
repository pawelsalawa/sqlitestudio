#include "exportworker.h"
#include "plugins/exportplugin.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include "common/utils.h"
#include <QMutexLocker>
#include <QDebug>

ExportWorker::ExportWorker(ExportPlugin* plugin, ExportManager::StandardExportConfig* config, QIODevice* output, QObject *parent) :
    QObject(parent), plugin(plugin), config(config), output(output)
{
    executor = new QueryExecutor();
    executor->setAsyncMode(false); // worker runs in its own thread, we don't need (and we don't want) async executor
    executor->setNoMetaColumns(true); // we don't want rowid columns, we want only columns requested by the original query
}

ExportWorker::~ExportWorker()
{
    safe_delete(executor);
    safe_delete(parser);
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
        case ExportManager::FILE:
        case ExportManager::CLIPBOARD:
            break;
    }

    plugin->cleanupAfterExport();

    emit finished(res, output);
}

void ExportWorker::prepareExportQueryResults(Db* db, const QString& query)
{
    this->db = db;
    this->query = query;
    exportMode = ExportManager::QUERY_RESULTS;
    prepareParser();
}

void ExportWorker::prepareExportDatabase(Db* db, const QStringList& objectListToExport)
{
    this->db = db;
    this->objectListToExport = objectListToExport;
    exportMode = ExportManager::DATABASE;
    prepareParser();
}

void ExportWorker::prepareExportTable(Db* db, const QString& database, const QString& table)
{
    this->db = db;
    this->database = database;
    this->table = table;
    exportMode = ExportManager::TABLE;
    prepareParser();
}

void ExportWorker::prepareParser()
{
    safe_delete(parser);
    parser = new Parser(db->getDialect());
}

void ExportWorker::interrupt()
{
    QMutexLocker locker(&interruptMutex);
    interrupted = true;
    if (executor->isExecutionInProgress())
        executor->interrupt();
}

bool ExportWorker::exportQueryResults()
{
    executor->setDb(db);
    executor->exec(query);
    SqlQueryPtr results = executor->getResults();
    if (!results)
    {
        qCritical() << "Null results from executor in ExportWorker.";
        return false;
    }

    QList<QueryExecutor::ResultColumnPtr> resultColumns = executor->getResultColumns();
    QHash<ExportManager::ExportProviderFlag,QVariant> providerData = getProviderDataForQueryResults();

    if (results->isInterrupted())
        return false;

    if (results->isError())
    {
        notifyError(tr("Error while exporting query results: %1").arg(results->getErrorText()));
        return false;
    }

    if (!plugin->initBeforeExport(db, output, *config))
        return false;

    if (!plugin->beforeExportQueryResults(query, resultColumns, providerData))
        return false;

    if (isInterrupted())
        return false;

    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        if (!plugin->exportQueryResultsRow(row))
            return false;

        if (isInterrupted())
            return false;
    }

    if (!plugin->afterExportQueryResults())
        return false;

    if (!plugin->afterExport())
        return false;

    return true;
}

QHash<ExportManager::ExportProviderFlag, QVariant> ExportWorker::getProviderDataForQueryResults()
{
    static const QString colLengthSql = QStringLiteral("SELECT %1 FROM (%2)");
    static const QString colLengthTpl = QStringLiteral("max(length(%1))");
    QHash<ExportManager::ExportProviderFlag, QVariant> providerData;

    if (plugin->getProviderFlags().testFlag(ExportManager::ROW_COUNT))
    {
        executor->countResults();
        providerData[ExportManager::ROW_COUNT] = executor->getTotalRowsReturned();
    }

    if (plugin->getProviderFlags().testFlag(ExportManager::DATA_LENGTHS))
    {
        QStringList wrappedCols;
        for (const QueryExecutor::ResultColumnPtr& col : executor->getResultColumns())
            wrappedCols << colLengthTpl.arg(wrapObjIfNeeded(col->displayName, db->getDialect()));

        executor->exec(colLengthSql.arg(wrappedCols.join(", "), query));
        SqlQueryPtr results = executor->getResults();
        if (!results)
        {
            qCritical() << "Null results from executor in ExportWorker.";
        }
        else if (results->isError())
        {
            notifyError(tr("Error while counting data column width to export from query results: %2").arg(results->getErrorText()));
        }
        else
        {
            QList<int> colWidths;
            for (const QVariant& value : results->next()->valueList())
                colWidths << value.toInt();

            providerData[ExportManager::DATA_LENGTHS] = QVariant::fromValue(colWidths);
        }
    }
    return providerData;
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

    if (!plugin->initBeforeExport(db, output, *config))
        return false;

    if (!plugin->beforeExportDatabase(db->getName()))
        return false;

    if (isInterrupted())
        return false;

    QList<ExportManager::ExportObject::Type> order = {
        ExportManager::ExportObject::TABLE, ExportManager::ExportObject::INDEX, ExportManager::ExportObject::TRIGGER, ExportManager::ExportObject::VIEW
    };

    qSort(dbObjects.begin(), dbObjects.end(), [=](const ExportManager::ExportObjectPtr& dbObj1, const ExportManager::ExportObjectPtr& dbObj2) -> bool
    {
        return order.indexOf(dbObj1->type) < order.indexOf(dbObj2->type);
    });

    if (!plugin->beforeExportTables())
        return false;

    if (!exportDatabaseObjects(dbObjects, ExportManager::ExportObject::TABLE))
        return false;

    if (!plugin->afterExportTables())
        return false;

    if (!plugin->beforeExportIndexes())
        return false;

    if (!exportDatabaseObjects(dbObjects, ExportManager::ExportObject::INDEX))
        return false;

    if (!plugin->afterExportIndexes())
        return false;

    if (!plugin->beforeExportTriggers())
        return false;

    if (!exportDatabaseObjects(dbObjects, ExportManager::ExportObject::TRIGGER))
        return false;

    if (!plugin->afterExportTriggers())
        return false;

    if (!plugin->beforeExportViews())
        return false;

    if (!exportDatabaseObjects(dbObjects, ExportManager::ExportObject::VIEW))
        return false;

    if (!plugin->afterExportViews())
        return false;

    if (!plugin->afterExportDatabase())
        return false;

    if (!plugin->afterExport())
        return false;

    return true;
}

bool ExportWorker::exportDatabaseObjects(const QList<ExportManager::ExportObjectPtr>& dbObjects, ExportManager::ExportObject::Type type)
{
    SqliteQueryPtr parsedQuery;
    bool res = true;
    for (const ExportManager::ExportObjectPtr& obj : dbObjects)
    {
        if (obj->type != type)
            continue;

        res = parser->parse(obj->ddl);
        if (!res || parser->getQueries().size() < 1)
        {
            qCritical() << "Could not parse" << obj->name << ", the DDL was:" << obj->ddl << ", error is:" << parser->getErrorString();
            notifyWarn(tr("Could not parse %1 in order to export it. It will be excluded from the export output.").arg(obj->name));
            continue;
        }
        parsedQuery = parser->getQueries().first();

        switch (obj->type)
        {
            case ExportManager::ExportObject::TABLE:
                res = exportTableInternal(obj->database, obj->name, obj->ddl, parsedQuery, obj->data, obj->providerData);
                break;
            case ExportManager::ExportObject::INDEX:
                res = plugin->exportIndex(obj->database, obj->name, obj->ddl, parsedQuery.dynamicCast<SqliteCreateIndex>());
                break;
            case ExportManager::ExportObject::TRIGGER:
                res = plugin->exportTrigger(obj->database, obj->name, obj->ddl, parsedQuery.dynamicCast<SqliteCreateTrigger>());
                break;
            case ExportManager::ExportObject::VIEW:
                res = plugin->exportView(obj->database, obj->name, obj->ddl, parsedQuery.dynamicCast<SqliteCreateView>());
                break;
            default:
                qDebug() << "Unhandled ExportObject type:" << obj->type;
                break;
        }

        if (!res)
            return false;

        if (isInterrupted())
            return false;
    }
    return true;
}

bool ExportWorker::exportTable()
{
    SqlQueryPtr results;
    QString errorMessage;
    QHash<ExportManager::ExportProviderFlag,QVariant> providerData;
    queryTableDataToExport(db, table, results, providerData, &errorMessage);
    if (!errorMessage.isNull())
    {
        notifyError(errorMessage);
        return false;
    }

    SchemaResolver resolver(db);
    QString ddl = resolver.getObjectDdl(database, table);

    if (!parser->parse(ddl) || parser->getQueries().size() < 1)
    {
        qCritical() << "Could not parse" << table << ", the DDL was:" << ddl << ", error is:" << parser->getErrorString();
        notifyWarn(tr("Could not parse %1 in order to export it. It will be excluded from the export output.").arg(table));
        return false;
    }

    SqliteQueryPtr createTable = parser->getQueries().first();
    if (!plugin->initBeforeExport(db, output, *config))
        return false;

    if (!exportTableInternal(database, table, ddl, createTable, results, providerData))
        return false;

    if (config->exportTableIndexes)
    {
        if (!plugin->beforeExportIndexes())
            return false;

        QList<SqliteCreateIndexPtr> parsedIndexesForTable = resolver.getParsedIndexesForTable(database, table);
        for (const SqliteCreateIndexPtr& idx : parsedIndexesForTable)
        {
            if (!plugin->exportIndex(database, idx->index, idx->detokenize(), idx))
                return false;
        }

        if (!plugin->afterExportIndexes())
            return false;
    }

    if (config->exportTableTriggers)
    {
        if (!plugin->beforeExportTriggers())
            return false;

        QList<SqliteCreateTriggerPtr> parsedTriggersForTable = resolver.getParsedTriggersForTable(database, table);
        for (const SqliteCreateTriggerPtr& trig : parsedTriggersForTable)
        {
            if (!plugin->exportTrigger(database, trig->trigger, trig->detokenize(), trig))
                return false;
        }

        if (!plugin->afterExportTriggers())
            return false;
    }

    if (!plugin->afterExport())
        return false;

    return true;
}

bool ExportWorker::exportTableInternal(const QString& database, const QString& table, const QString& ddl, SqliteQueryPtr parsedDdl, SqlQueryPtr results,
                                       const QHash<ExportManager::ExportProviderFlag,QVariant>& providerData)
{
    SqliteCreateTablePtr createTable = parsedDdl.dynamicCast<SqliteCreateTable>();
    SqliteCreateVirtualTablePtr createVirtualTable = parsedDdl.dynamicCast<SqliteCreateVirtualTable>();

    if (createTable)
    {
        if (!plugin->exportTable(database, table, results->getColumnNames(), ddl, createTable, providerData))
            return false;
    }
    else
    {
        if (!plugin->exportVirtualTable(database, table, results->getColumnNames(), ddl, createVirtualTable, providerData))
            return false;
    }

    if (isInterrupted())
        return false;

    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        if (!plugin->exportTableRow(row))
            return false;

        if (isInterrupted())
            return false;
    }

    if (!plugin->afterExportTable())
        return false;

    return true;
}

QList<ExportManager::ExportObjectPtr> ExportWorker::collectDbObjects(QString* errorMessage)
{
    SchemaResolver resolver(db);
    QHash<QString, SchemaResolver::ObjectDetails> allDetails = resolver.getAllObjectDetails();

    QList<ExportManager::ExportObjectPtr> objectsToExport;
    ExportManager::ExportObjectPtr exportObj;
    SchemaResolver::ObjectDetails details;
    for (const QString& objName : objectListToExport)
    {
        if (!allDetails.contains(objName))
        {
            qWarning() << "Object requested for export, but not on list of db object details:" << objName;
            continue;
        }
        details = allDetails[objName];

        exportObj = ExportManager::ExportObjectPtr::create();
        if (details.type == SchemaResolver::ObjectDetails::TABLE)
        {
            exportObj->type = ExportManager::ExportObject::TABLE;
            queryTableDataToExport(db, objName, exportObj->data, exportObj->providerData, errorMessage);
            if (!errorMessage->isNull())
                return objectsToExport;
        }
        else if (details.type == SchemaResolver::ObjectDetails::INDEX)
            exportObj->type = ExportManager::ExportObject::INDEX;
        else if (details.type == SchemaResolver::ObjectDetails::TRIGGER)
            exportObj->type = ExportManager::ExportObject::TRIGGER;
        else if (details.type == SchemaResolver::ObjectDetails::VIEW)
            exportObj->type = ExportManager::ExportObject::VIEW;
        else
            continue; // warning about this case is already in SchemaResolver

        exportObj->name = objName;
        exportObj->ddl = details.ddl;
        objectsToExport << exportObj;
    }

    qSort(objectsToExport.begin(), objectsToExport.end(), [](ExportManager::ExportObjectPtr obj1, ExportManager::ExportObjectPtr obj2) -> bool
    {
        return (obj1->type < obj2->type);
    });

    return objectsToExport;
}

void ExportWorker::queryTableDataToExport(Db* db, const QString& table, SqlQueryPtr& dataPtr, QHash<ExportManager::ExportProviderFlag,QVariant>& providerData,
                                          QString* errorMessage) const
{
    static const QString sql = QStringLiteral("SELECT * FROM %1");
    static const QString countSql = QStringLiteral("SELECT count(*) FROM %1");
    static const QString colLengthSql = QStringLiteral("SELECT %1 FROM %2");
    static const QString colLengthTpl = QStringLiteral("max(length(%1))");

    if (config->exportData)
    {
        QString wrappedTable = wrapObjIfNeeded(table, db->getDialect());

        dataPtr = db->exec(sql.arg(wrappedTable));
        if (dataPtr->isError())
            *errorMessage = tr("Error while reading data to export from table %1: %2").arg(table, dataPtr->getErrorText());

        if (plugin->getProviderFlags().testFlag(ExportManager::ROW_COUNT))
        {
            SqlQueryPtr countQuery = db->exec(countSql.arg(wrappedTable));
            if (countQuery->isError())
            {
                if (errorMessage->isNull())
                    *errorMessage = tr("Error while counting data to export from table %1: %2").arg(table, dataPtr->getErrorText());
            }
            else
                providerData[ExportManager::ROW_COUNT] = countQuery->getSingleCell().toInt();
        }

        if (plugin->getProviderFlags().testFlag(ExportManager::DATA_LENGTHS))
        {
            QStringList wrappedCols;
            for (const QString& col : dataPtr->getColumnNames())
                wrappedCols << colLengthTpl.arg(wrapObjIfNeeded(col, db->getDialect()));

            SqlQueryPtr colLengthQuery = db->exec(colLengthSql.arg(wrappedCols.join(", "), wrappedTable));
            if (colLengthQuery->isError())
            {
                if (errorMessage->isNull())
                    *errorMessage = tr("Error while counting data column width to export from table %1: %2").arg(table, dataPtr->getErrorText());
            }
            else
            {
                QList<int> colWidths;
                for (const QVariant& value : colLengthQuery->next()->valueList())
                    colWidths << value.toInt();

                providerData[ExportManager::DATA_LENGTHS] = QVariant::fromValue(colWidths);
            }
        }
    }
}

bool ExportWorker::isInterrupted()
{
    QMutexLocker locker(&interruptMutex);
    return interrupted;
}

