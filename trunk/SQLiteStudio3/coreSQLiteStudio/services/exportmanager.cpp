#include "exportmanager.h"
#include "services/pluginmanager.h"
#include "plugins/exportplugin.h"
#include "services/notifymanager.h"
#include "db/queryexecutor.h"
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFile>

ExportManager::ExportManager(QObject *parent) :
    QObject(parent)
{
    executor = new QueryExecutor();
}

ExportManager::~ExportManager()
{
    safe_delete(executor);
    safe_delete(config);
}

QStringList ExportManager::getAvailableFormats() const
{
    QStringList formats;
    for (ExportPlugin* plugin : PLUGINS->getLoadedPlugins<ExportPlugin>())
        formats << plugin->getFormatName();

    return formats;
}

void ExportManager::configure(const QString& format, StandardExportConfig* config)
{
    if (exportInProgress)
    {
        qWarning() << "Tried to configure export while another export is in progress.";
        return;
    }

    plugin = getPluginForFormat(format);
    if (!plugin)
    {
        invalidFormat(format);
        return;
    }

    safe_delete(this->config);
    this->config = config;
}

bool ExportManager::isExportInProgress() const
{
    return exportInProgress;
}

void ExportManager::exportQueryResults(Db* db, const QString& query)
{
    if (!checkInitialConditions())
        return;

    exportInProgress = true;
    mode = RESULTS;

    executor->setDb(db);
    executor->setQuery(query);
    executor->exec([=](SqlResultsPtr results)
    {
        this->processExportQueryResults(db, query, results);
    });
}

void ExportManager::exportTable(Db* db, const QString& database, const QString& table)
{
    if (!checkInitialConditions())
        return;
}

void ExportManager::exportDatabase(Db* db)
{
    if (!checkInitialConditions())
        return;
}

ExportPlugin* ExportManager::getPluginForFormat(const QString& formatName) const
{
    for (ExportPlugin* plugin : PLUGINS->getLoadedPlugins<ExportPlugin>())
        if (plugin->getFormatName() == formatName)
            return plugin;

    return nullptr;
}

void ExportManager::invalidFormat(const QString& format)
{
    notifyError(tr("Export format '%1' is not supported. Supported formats are: %2.").arg(format).arg(getAvailableFormats().join(", ")));
}

bool ExportManager::checkInitialConditions()
{
    if (exportInProgress)
    {
        qWarning() << "Tried to call export while another export is in progress.";
        return false;
    }

    if (!plugin)
    {
        qWarning() << "Tried to call export while no export plugin was configured.";
        return false;
    }

    return true;
}

void ExportManager::processExportQueryResults(Db* db, const QString& query, SqlResultsPtr results)
{
    QList<QueryExecutor::ResultColumnPtr> resultColumns = executor->getResultColumns();
    QIODevice* output = getOutputStream();
    if (!output)
    {
        exportInProgress = false;
        return;
    }

    bool res = plugin->exportQueryResults(db, query, results, resultColumns, output, *config);
    output->close();
    delete output;

    if (res)
    {
        if (config->intoClipboard)
            notifyInfo(tr("Query results successfly exported to the clipboard."));
        else
            notifyInfo(tr("Query results successfly exported to file: %1").arg(config->outputFileName));
    }

    exportInProgress = false;
}

QIODevice* ExportManager::getOutputStream()
{
    if (config->intoClipboard)
    {
        QBuffer* buffer = new QBuffer();
        buffer->open(QIODevice::WriteOnly);
        return buffer;
    }
    else if (!config->outputFileName.trimmed().isEmpty())
    {
        QFile* file = new QFile(config->outputFileName);
        if (!file->open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            notifyError(tr("Could not export to file %1. File cannot be open for writting.").arg(config->outputFileName));
            delete file;
            return nullptr;
        }
        return file;
    }
    return nullptr;
}
