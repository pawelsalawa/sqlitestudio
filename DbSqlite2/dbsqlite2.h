#ifndef DBSQLITE2_H
#define DBSQLITE2_H

#include "dbsqlite2_global.h"
#include "plugins/plugin.h"
#include "plugins/dbpluginqt.h"

class DBSQLITE2SHARED_EXPORT DbSqlite2 : public DbPluginQt
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

    protected:
        DbQt* getInstance();
        QString getDriver();
};

#endif // DBSQLITE2_H
