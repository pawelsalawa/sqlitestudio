#ifndef DBSQLITE2_H
#define DBSQLITE2_H

#include "dbsqlite2_global.h"
#include "plugins/dbplugin.h"
#include "plugins/genericplugin.h"

class DBSQLITE2SHARED_EXPORT DbSqlite2 : public GenericPlugin, public DbPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN
    SQLITESTUDIO_PLUGIN_TITLE("SQLite 2")
    SQLITESTUDIO_PLUGIN_DESC("Provides support for SQLite 2.* databases")
    SQLITESTUDIO_PLUGIN_VERSION(10000)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        DbSqlite2();

        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        Db* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage);
        QList<DbPluginOption> getOptionsList() const;
        QString generateDbName(const QVariant& baseValue);
};

#endif // DBSQLITE2_H
