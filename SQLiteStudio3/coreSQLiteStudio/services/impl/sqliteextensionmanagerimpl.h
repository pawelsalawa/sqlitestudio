#ifndef SQLITEEXTENSIONMANAGERIMPL_H
#define SQLITEEXTENSIONMANAGERIMPL_H

#include "services/sqliteextensionmanager.h"

class SqliteExtensionManagerImpl : public SqliteExtensionManager
{
    public:
        SqliteExtensionManagerImpl();

        void setExtensions(const QList<ExtensionPtr>& newExtensions);
        QList<ExtensionPtr> getAllExtensions() const;
        QList<ExtensionPtr> getExtensionForDatabase(const QString& dbName) const;

    private:
        void init();
        void storeInConfig();
        void loadFromConfig();

        QList<ExtensionPtr> extensions;
};

#endif // SQLITEEXTENSIONMANAGERIMPL_H
