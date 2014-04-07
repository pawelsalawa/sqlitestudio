#include "sqlexport.h"

CFG_DEFINE(SqlExportConfig)

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
    return ExportManager::DATABASE|ExportManager::TABLE|ExportManager::RESULTS;
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
    if (mode == ExportManager::RESULTS)
        return "sqlExportQueryConfig";

    return QString::null;
}

bool SqlExport::exportQueryResults(Db* db, const QString& query, SqlResultsPtr results, QList<QueryExecutor::ResultColumnPtr>& columns, QIODevice* output,
                                   const ExportManager::StandardExportConfig& config)
{
    return true;
}

bool SqlExport::exportTable(Db* db, const QString& database, const QString& table, const QString& ddl, SqlResultsPtr data, QIODevice* output,
                            const ExportManager::StandardExportConfig& config)
{
    return true;
}

bool SqlExport::exportDatabase(Db* db, const QList<ExportManager::ExportObject>& objectsToExport, QIODevice* output, const ExportManager::StandardExportConfig& config)
{
    return true;
}
