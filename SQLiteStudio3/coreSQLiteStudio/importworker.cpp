#include "importworker.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "db/db.h"
#include "plugins/importplugin.h"
#include "common/utils.h"

ImportWorker::ImportWorker(ImportPlugin* plugin, ImportManager::StandardImportConfig* config, Db* db, const QString& table, QObject *parent) :
    QObject(parent), plugin(plugin), config(config), db(db), table(table)
{
}

void ImportWorker::run()
{
    if (!plugin->beforeImport(*config))
    {
        emit finished(false, 0);
        return;
    }

    readPluginColumns();
    if (columnsFromPlugin.size() == 0)
    {
        error(tr("No columns provided by the import plugin."));
        return;
    }

    shouldSkipTransaction = config->skipTransaction || db->isTransactionActive();
    if (!shouldSkipTransaction && !db->begin(config->noDbLock))
    {
        error(tr("Could not start transaction in order to import a data: %1").arg(db->getErrorText()));
        return;
    }

    if (!prepareTable())
    {
        if (!shouldSkipTransaction)
            db->rollback(config->noDbLock);

        return;
    }

    int rowCount = 0;
    if (!importData(rowCount))
    {
        if (!shouldSkipTransaction)
            db->rollback(config->noDbLock);

        return;
    }

    if (!shouldSkipTransaction && !db->commit(config->noDbLock))
    {
        error(tr("Could not commit transaction for imported data: %1").arg(db->getErrorText()));
        if (!shouldSkipTransaction)
            db->rollback(config->noDbLock);

        return;
    }

    if (tableCreated)
        emit createdTable(db, table);

    plugin->afterImport();
    emit finished(true, rowCount);
}

void ImportWorker::interrupt()
{
    QMutexLocker locker(&interruptMutex);
    interrupted = true;
}

void ImportWorker::readPluginColumns()
{
    QList<ImportPlugin::ColumnDefinition> pluginColumnDefinitions = plugin->getColumns();
    for (const ImportPlugin::ColumnDefinition& colDef : pluginColumnDefinitions)
    {
        columnsFromPlugin << colDef.first;
        columnTypesFromPlugin << colDef.second;
    }
}

void ImportWorker::error(const QString& err)
{
    notifyError(err);
    plugin->afterImport();
    emit finished(false, 0);
}

bool ImportWorker::prepareTable()
{
    QStringList finalColumns;

    SchemaResolver resolver(db);
    tableColumns = resolver.getTableColumns(table);
    if (tableColumns.size() > 0)
    {
        if (tableColumns.size() < columnsFromPlugin.size())
        {
            notifyWarn(tr("Table '%1' has less columns than there are columns in the data to be imported. Excessive data columns will be ignored.").arg(table));
            finalColumns = tableColumns;
        }
        else if (tableColumns.size() > columnsFromPlugin.size())
        {
            notifyInfo(tr("Table '%1' has more columns than there are columns in the data to be imported. Some columns in the table will be left empty.").arg(table));
            finalColumns = tableColumns.mid(0, columnsFromPlugin.size());
        }
        else
        {
            finalColumns = tableColumns;
        }
    }
    else
    {
        QStringList colDefs;
        for (int i = 0; i < columnsFromPlugin.size(); i++)
            colDefs << (wrapObjIfNeeded(columnsFromPlugin[i]) + " " + columnTypesFromPlugin[i]).trimmed();

        static const QString ddl = QStringLiteral("CREATE TABLE %1 (%2)");
        Db::Flags flags = config->noDbLock ? Db::Flag::NO_LOCK : Db::Flag::NONE;
        SqlQueryPtr result = db->exec(ddl.arg(wrapObjIfNeeded(table), colDefs.join(", ")), flags);
        if (result->isError())
        {
            error(tr("Could not create table to import to: %1").arg(result->getErrorText()));
            return false;
        }
        finalColumns = columnsFromPlugin;
        tableCreated = true;
    }

    if (isInterrupted())
    {
        error(tr("Error while importing data: %1").arg(tr("Interrupted.", "import process status update")));
        return false;
    }

    targetColumns = wrapObjNamesIfNeeded(finalColumns);
    return true;
}

bool ImportWorker::importData(int& rowCount)
{
    static const QString insertTemplate = QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)");

    int colCount = targetColumns.size();
    QStringList valList;
    for (int i = 0; i < colCount; i++)
        valList << "?";

    QString theInsert = insertTemplate.arg(wrapObjIfNeeded(table),
                                           targetColumns.join(", "),
                                           valList.join(", "));

    SqlQueryPtr query = db->prepare(theInsert);
    query->setFlags(Db::Flag::SKIP_DROP_DETECTION|Db::Flag::SKIP_PARAM_COUNTING|Db::Flag::NO_LOCK);

    rowCount = 0;
    QList<QVariant> row;
    while ((row = plugin->next()).size() > 0)
    {
        // Fill up missing values in the line
        for (int i = row.size(); i < colCount; i++)
            row << QVariant(QMetaType::fromType<QString>());

        // Assign argument values
        query->setArgs(row.mid(0, colCount));

        if (!query->execute())
        {
            if (config->ignoreErrors)
            {
                qDebug() << "Could not import data row number" << (rowCount+1) << ". The row was ignored. Problem details:"
                         << query->getErrorText();

                notifyWarn(tr("Could not import data row number %1. The row was ignored. Problem details: %2")
                           .arg(QString::number(rowCount + 1), query->getErrorText()));
            }
            else
            {
                error(tr("Error while importing data: %1").arg(query->getErrorText()));
                return false;
            }
        }

        if ((rowCount % 100) == 0 && isInterrupted())
        {
            error(tr("Error while importing data: %1").arg(tr("Interrupted.", "import process status update")));
            return false;
        }
        rowCount++;
    }

    return true;
}

bool ImportWorker::isInterrupted()
{
    QMutexLocker locker(&interruptMutex);
    return interrupted;
}
