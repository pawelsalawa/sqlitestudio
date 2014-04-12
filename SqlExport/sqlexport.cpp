#include "sqlexport.h"

CFG_DEFINE_RUNTIME(SqlExportConfig)

QString SqlExport::getFormatName() const
{
    return "SQL";
}

ExportManager::StandardConfigFlags SqlExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

ExportManager::ExportModes SqlExport::getSupportedModes() const
{
    return ExportManager::DATABASE|ExportManager::TABLE|ExportManager::QUERY_RESULTS;
}

CfgMain* SqlExport::getConfig() const
{
    return &SQL_EXPORT_CFG;
}

QString SqlExport::defaultFileExtension() const
{
    return ".sql";
}

QString SqlExport::getConfigFormName(ExportManager::ExportMode mode) const
{
    if (mode == ExportManager::QUERY_RESULTS)
        return "sqlExportQueryConfig";

    return QString::null;
}

void SqlExport::initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config)
{
}

bool SqlExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns)
{
    return true;
}

bool SqlExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    return true;
}

bool SqlExport::afterExportQueryResults()
{
    return true;
}

bool SqlExport::beforeExportTable(const QString& database, const QString& table, const QString& ddl, bool databaseExport)
{
    return true;
}

bool SqlExport::exportTableRow(SqlResultsRowPtr data)
{
    return true;
}

bool SqlExport::afterExportTable()
{
    return true;
}

bool SqlExport::beforeExportDatabase()
{
    return true;
}

bool SqlExport::exportIndex(const QString& database, const QString& name, const QString& ddl)
{
    return true;
}

bool SqlExport::exportTrigger(const QString& database, const QString& name, const QString& ddl)
{
    return true;
}

bool SqlExport::exportView(const QString& database, const QString& name, const QString& ddl)
{
    return true;
}

bool SqlExport::afterExportDatabase()
{
    return true;
}
