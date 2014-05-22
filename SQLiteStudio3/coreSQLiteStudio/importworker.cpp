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
        emit finished(false);
        return;
    }

    readPluginColumns();
    if (columnsFromPlugin.size() == 0)
    {
        error(tr("No columns provided by the import plugin."));
        return;
    }

    if (!db->begin())
    {
        error(tr("Could not start transaction in order to import a data: %1").arg(db->getErrorText()));
        return;
    }

    if (!prepareTable())
    {
        db->rollback();
        return;
    }

    if (!importData())
    {
        db->rollback();
        return;
    }

    if (!db->commit())
    {
        error(tr("Could not commit transaction for imported data: %1").arg(db->getErrorText()));
        db->rollback();
        return;
    }

    emit finished(true);
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
    emit finished(false);
}

bool ImportWorker::prepareTable()
{
    QStringList finalColumns;
    Dialect dialect = db->getDialect();

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
            colDefs << columnsFromPlugin[i] << " " << columnTypesFromPlugin[i];

        static const QString ddl = QStringLiteral("CREATE TABLE %1 (%2)");
        SqlQueryPtr result = db->exec(ddl.arg(wrapObjIfNeeded(table, dialect), colDefs.join(", ")));
        if (result->isError())
        {
            error(tr("Could not create table to import to: %1").arg(result->getErrorText()));
            return false;
        }
        finalColumns = columnsFromPlugin;
    }

    if (isInterrupted())
    {
        error(tr("Error while importing data: %1").arg(tr("Interrupted.", "import process status update")));
        return false;
    }

    targetColumns = wrapObjNamesIfNeeded(finalColumns, dialect);
    return true;
}

bool ImportWorker::importData()
{
    static const QString insertTemplate = QStringLiteral("INSERT INTO %1 VALUES (%2)");

    int colCount = targetColumns.size();
    QStringList valList;
    for (int i = 0; i < colCount; i++)
        valList << "?";

    QString theInsert = insertTemplate.arg(wrapObjIfNeeded(table, db->getDialect()), valList.join(", "));
    SqlQueryPtr query = db->prepare(theInsert);

    int rowCnt = 0;
    QList<QVariant> row;
    while ((row = plugin->next()).size() > 0)
    {
        query->setArgs(row.mid(0, colCount));
        if (!query->execute())
        {
            error(tr("Error while importing data: %1").arg(query->getErrorText()));
            return false;
        }

        if ((rowCnt % 100) == 0 && isInterrupted())
        {
            error(tr("Error while importing data: %1").arg(tr("Interrupted.", "import process status update")));
            return false;
        }
        rowCnt++;
    }

    return true;
}

bool ImportWorker::isInterrupted()
{
    QMutexLocker locker(&interruptMutex);
    return interrupted;
}
