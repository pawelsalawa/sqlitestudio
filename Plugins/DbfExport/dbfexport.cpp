#include "dbfexport.h"
#include "common/unused.h"

DbfExport::DbfExport()
{
}

QString DbfExport::getFormatName() const
{
    return "DBF (DBase)";
}

ExportManager::StandardConfigFlags DbfExport::standardOptionsToEnable() const
{
    return 0;
}

bool DbfExport::isBinaryData() const
{
    return true;
}

ExportManager::ExportModes DbfExport::getSupportedModes() const
{
    return ExportManager::TABLE|ExportManager::QUERY_RESULTS;
}

QString DbfExport::getExportConfigFormName() const
{
    return QString();
}

CfgMain* DbfExport::getConfig()
{
    return nullptr;
}

void DbfExport::validateOptions()
{
}

QString DbfExport::defaultFileExtension() const
{
    return "dbf";
}

bool DbfExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    return true;
}

bool DbfExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    return true;
}

bool DbfExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    return true;
}

bool DbfExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    return true;
}

bool DbfExport::exportTableRow(SqlResultsRowPtr data)
{
    return true;
}

bool DbfExport::beforeExportDatabase(const QString& database)
{
    UNUSED(database);
    return false;
}

bool DbfExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    return isTableExport();
}

bool DbfExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    return isTableExport();
}

bool DbfExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView)
{
    return false;
}

bool DbfExport::init()
{
    return true;
}

void DbfExport::deinit()
{

}

bool DbfExport::exportTable(const QStringList& columnNames)
{
    dbfTable = new QDbf::QDbfTable();
    return true;
}
