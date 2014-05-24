#include "importmanager.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "plugins/importplugin.h"
#include "importworker.h"
#include "db/db.h"
#include "common/unused.h"
#include <QThreadPool>
#include <QDebug>

ImportManager::ImportManager()
{
}

QStringList ImportManager::getImportDataSourceTypes() const
{
    QStringList types;
    for (ImportPlugin* plugin : PLUGINS->getLoadedPlugins<ImportPlugin>())
        types << plugin->getDataSourceTypeName();

    return types;
}

ImportPlugin* ImportManager::getPluginForDataSourceType(const QString& dataSourceType) const
{
    for (ImportPlugin* plugin : PLUGINS->getLoadedPlugins<ImportPlugin>())
    {
        if (plugin->getDataSourceTypeName() == dataSourceType)
            return plugin;
    }

    return nullptr;
}

void ImportManager::configure(const QString& dataSourceType, const ImportManager::StandardImportConfig& config)
{
    plugin = getPluginForDataSourceType(dataSourceType);
    importConfig = config;
}

void ImportManager::importToTable(Db* db, const QString& table)
{
    this->db = db;
    this->table = table;

    if (!db->isOpen())
    {
        emit importFailed();
        qCritical() << "Tried to import into closed database.";
        return;
    }

    if (!plugin)
    {
        emit importFailed();
        qCritical() << "Tried to import, while ImportPlugin was null.";
        return;
    }

    importInProgress = true;

    ImportWorker* worker = new ImportWorker(plugin, &importConfig, db, table);
    connect(worker, SIGNAL(finished(bool)), this, SLOT(finalizeImport(bool)));
    connect(worker, SIGNAL(createdTable(Db*,QString)), this, SLOT(handleTableCreated(Db*,QString)));
    connect(this, SIGNAL(orderWorkerToInterrup()), worker, SLOT(interrupt()));

    QThreadPool::globalInstance()->start(worker);
}

void ImportManager::handleValidationFromPlugin(bool configValid, CfgEntry* key, const QString& errorMessage)
{
    emit validationResultFromPlugin(configValid, key, errorMessage);
}

void ImportManager::updateVisibilityAndEnabled(CfgEntry* key, bool visible, bool enabled)
{
    emit stateUpdateRequestFromPlugin(key, visible, enabled);
}

void ImportManager::interrupt()
{
    emit orderWorkerToInterrup();
}

bool ImportManager::isAnyPluginAvailable()
{
    return PLUGINS->getLoadedPlugins<ImportPlugin>().size() > 0;
}

void ImportManager::finalizeImport(bool result)
{
    importInProgress = false;
    emit importFinished();
    if (result)
    {
        notifyInfo(tr("Imported data to the table '%1' successfully.").arg(table));
        emit importSuccessful();
    }
    else
        emit importFailed();
}

void ImportManager::handleTableCreated(Db* db, const QString& table)
{
    UNUSED(table);
    emit schemaModified(db);
}
