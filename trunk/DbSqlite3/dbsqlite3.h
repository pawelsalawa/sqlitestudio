#ifndef DBSQLITE3_H
#define DBSQLITE3_H

#include "dbsqlite3_global.h"
#include "plugins/plugin.h"
#include "plugins/dbpluginqt.h"

class DBSQLITE3SHARED_EXPORT DbSqlite3 : public DbPluginQt
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN
    SQLITESTUDIO_PLUGIN_TITLE("SQLite 3")
    SQLITESTUDIO_PLUGIN_DESC("Provides support for SQLite 3.* databases")
    SQLITESTUDIO_PLUGIN_VERSION(10000)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        DbSqlite3();
        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;

    protected:
        DbQt* getInstance();
        QString getDriver();
};

#endif // DBSQLITE3_H
