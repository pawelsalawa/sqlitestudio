#ifndef DBPLUGINSQLITE3_H
#define DBPLUGINSQLITE3_H

#include "dbplugin.h"
#include "builtinplugin.h"

class DbPluginSqlite3 : public BuiltInPlugin, public DbPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_TITLE("SQLite 3")
    SQLITESTUDIO_PLUGIN_DESC("SQLite 3 databases support.")
    SQLITESTUDIO_PLUGIN_VERSION(10001)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        Db* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage);
        QString getLabel() const;
        QList<DbPluginOption> getOptionsList() const;
        QString generateDbName(const QVariant& baseValue);
        bool checkIfDbServedByPlugin(Db* db) const;
        bool usesPath() const;
};

#endif // DBPLUGINSQLITE3_H
