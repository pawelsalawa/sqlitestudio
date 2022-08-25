#include "extensionmanagermock.h"

ExtensionManagerMock::ExtensionManagerMock()
{
}

void ExtensionManagerMock::setExtensions(const QList<SqliteExtensionManager::ExtensionPtr>&)
{
}

QList<SqliteExtensionManager::ExtensionPtr> ExtensionManagerMock::getAllExtensions() const
{
    return QList<SqliteExtensionManager::ExtensionPtr>();
}

QList<SqliteExtensionManager::ExtensionPtr> ExtensionManagerMock::getExtensionForDatabase(const QString&) const
{
    return QList<SqliteExtensionManager::ExtensionPtr>();
}

QStringList ExtensionManagerMock::getExtensionDirs() const
{
    return QStringList();
}
