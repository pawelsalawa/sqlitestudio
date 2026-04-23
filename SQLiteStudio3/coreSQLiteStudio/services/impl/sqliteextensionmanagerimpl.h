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
        QStringList getExtensionDirs() const;
        void loadFromConfig();
        void init();

    private:
        void scanExtensionDirs();
        void storeInConfig();

        QList<ExtensionPtr> extensions;
        QStringList extensionDirs;

    private slots:
        void handleDbUpdated(const QString& oldName, Db* db);
};

#endif // SQLITEEXTENSIONMANAGERIMPL_H
