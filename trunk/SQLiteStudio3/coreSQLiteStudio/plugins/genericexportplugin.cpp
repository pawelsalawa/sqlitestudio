#include "genericexportplugin.h"
#include "common/utils.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include "config_builder.h"
#include <QTextCodec>

void GenericExportPlugin::initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config)
{
    this->db = db;
    this->output = output;
    this->config = &config;

    if (standardOptionsToEnable().testFlag(ExportManager::CODEC))
    {
        codec = codecForName(this->config->codec);
        if (!codec)
        {
            codec = defaultCodec();
            notifyWarn(tr("Could not initialize text codec for exporting. Using default codec: %1").arg(QString::fromLatin1(codec->name())));
        }
    }
}

ExportManager::ExportModes GenericExportPlugin::getSupportedModes() const
{
    return ExportManager::CLIPBOARD|ExportManager::DATABASE|ExportManager::TABLE|ExportManager::QUERY_RESULTS;
}

CfgMain* GenericExportPlugin::getConfig()
{
    return nullptr;
}

QString GenericExportPlugin::getConfigFormName(ExportManager::ExportMode mode) const
{
    UNUSED(mode);
    return QString::null;
}

QString GenericExportPlugin::getMimeType() const
{
    return QString::null;
}

void GenericExportPlugin::setExportMode(ExportManager::ExportMode mode)
{
    this->exportMode = mode;
}

void GenericExportPlugin::initBeforeExport()
{
}

void GenericExportPlugin::write(const QString& str)
{
    output->write(codec->fromUnicode(str));
}

void GenericExportPlugin::writeln(const QString& str)
{
    write(str + "\n");
}
