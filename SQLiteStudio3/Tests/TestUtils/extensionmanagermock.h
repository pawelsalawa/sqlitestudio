#ifndef EXTENSIONMANAGERMOCK_H
#define EXTENSIONMANAGERMOCK_H

#include "services/sqliteextensionmanager.h"

class ExtensionManagerMock : public SqliteExtensionManager
{
    public:
        ExtensionManagerMock();

    public:
        void setExtensions(const QList<ExtensionPtr>&);
        QList<ExtensionPtr> getAllExtensions() const;
        QList<ExtensionPtr> getExtensionForDatabase(const QString&) const;
        QStringList getExtensionDirs() const;
};

#endif // EXTENSIONMANAGERMOCK_H
