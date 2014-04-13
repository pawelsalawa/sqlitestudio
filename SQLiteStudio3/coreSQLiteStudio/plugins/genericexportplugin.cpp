#include "genericexportplugin.h"

void GenericExportPlugin::initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config)
{
    this->db = db;
    this->output = output;
    this->config = &config;
}

ExportManager::ExportModes GenericExportPlugin::getSupportedModes() const
{
    return ExportManager::DATABASE|ExportManager::TABLE|ExportManager::QUERY_RESULTS;
}

CfgMain* GenericExportPlugin::getConfig() const
{
    return nullptr;
}

QString GenericExportPlugin::getConfigFormName(ExportManager::ExportMode mode) const
{
    return QString::null;
}
