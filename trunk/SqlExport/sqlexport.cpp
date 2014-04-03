#include "sqlexport.h"

QString SqlExport::getFormatName() const
{

}

ExportManager::StandardConfigFlags SqlExport::standardOptionsToEnable() const
{

}

ExportManager::ExportModes SqlExport::getSupportedModes() const
{

}

CfgMain* SqlExport::getConfig() const
{

}

QString SqlExport::defaultFileExtension() const
{

}

QString SqlExport::getConfigFormName(ExportManager::ExportMode mode) const
{

}

bool SqlExport::exportQueryResults(Db* db, const QString& query, SqlResultsPtr results, QList<QueryExecutor::ResultColumnPtr>& columns, QIODevice* output,
                                   const ExportManager::StandardExportConfig& config)
{

}

bool SqlExport::exportTable(Db* db, const QString& database, const QString& table, const QString& ddl, SqlResultsPtr data, QIODevice* output,
                            const ExportManager::StandardExportConfig& config)
{

}

bool SqlExport::exportDatabase(Db* db, const QList<ExportManager::ExportObject>& objectsToExport, QIODevice* output, const ExportManager::StandardExportConfig& config)
{

}
