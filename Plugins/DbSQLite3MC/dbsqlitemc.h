#ifndef DBSQLITEMC_H
#define DBSQLITEMC_H

#include "plugins/genericplugin.h"
#include "plugins/dbpluginstdfilebase.h"
#include "common/bistrhash.h"
#include <QObject>

class DbSqliteMc : public GenericPlugin, public DbPluginStdFileBase
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbsqlitemc.json")

    public:
        DbSqliteMc();

        static BiStrHash getCiphers();

        QString getLabel() const;
        QList<DbPluginOption> getOptionsList() const;
        bool checkIfDbServedByPlugin(Db *db) const;

        static_char* PASSWORD_OPT = "password";
        static_char* CIPHER_OPT = "cipher";
        static_char* PRAGMAS_OPT = "pragmas";

    protected:
        Db *newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options);
};

#endif // DBSQLITEMC_H
