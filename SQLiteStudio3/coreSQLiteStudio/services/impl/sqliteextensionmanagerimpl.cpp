#include "sqliteextensionmanagerimpl.h"
#include "services/notifymanager.h"
#include "services/dbmanager.h"

SqliteExtensionManagerImpl::SqliteExtensionManagerImpl()
{
    init();
}

void SqliteExtensionManagerImpl::setExtensions(const QList<SqliteExtensionManager::ExtensionPtr>& newExtensions)
{
    extensions = newExtensions;
    storeInConfig();
    emit extensionListChanged();
}

QList<SqliteExtensionManager::ExtensionPtr> SqliteExtensionManagerImpl::getAllExtensions() const
{
    return extensions;
}

QList<SqliteExtensionManager::ExtensionPtr> SqliteExtensionManagerImpl::getExtensionForDatabase(const QString& dbName) const
{
    QList<ExtensionPtr> results;
    for (const ExtensionPtr& ext : extensions)
    {
        if (ext->allDatabases || ext->databases.contains(dbName, Qt::CaseInsensitive))
            results << ext;
    }
    return results;
}

void SqliteExtensionManagerImpl::init()
{
    loadFromConfig();
}

void SqliteExtensionManagerImpl::storeInConfig()
{
    QVariantList list;
    QHash<QString,QVariant> extHash;
    for (ExtensionPtr ext : extensions)
    {
        extHash["filePath"] = ext->filePath;
        extHash["initFunc"] = ext->initFunc;
        extHash["allDatabases"] = ext->allDatabases;
        extHash["databases"] =common(DBLIST->getDbNames(),  ext->databases);
        list << extHash;
    }
    CFG_CORE.Internal.Extensions.set(list);
}

void SqliteExtensionManagerImpl::loadFromConfig()
{
    extensions.clear();

    QVariantList list = CFG_CORE.Internal.Extensions.get();
    QHash<QString,QVariant> extHash;
    ExtensionPtr ext;
    for (const QVariant& var : list)
    {
        extHash = var.toHash();
        ext = ExtensionPtr::create();
        ext->filePath = extHash["filePath"].toString();
        ext->initFunc = extHash["initFunc"].toString();
        ext->databases = extHash["databases"].toStringList();
        ext->allDatabases = extHash["allDatabases"].toBool();
        extensions << ext;
    }
}
