#ifndef DBSQLITECIPHER_H
#define DBSQLITECIPHER_H

#include "dbsqlitecipher_global.h"
#include "plugins/dbpluginstdfilebase.h"
#include "plugins/genericplugin.h"

class DBSQLITECIPHERSHARED_EXPORT DbSqliteCipher : public GenericPlugin, public DbPluginStdFileBase
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbsqlitecipher.json")

    public:
        DbSqliteCipher();

        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        QList<DbPluginOption> getOptionsList() const;
        bool init();
        void deinit();

        static_char* PASSWORD_OPT = "password";
        static_char* PRAGMAS_OPT = "pragmas";

    protected:
        Db *newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options);

    private:
        bool initValid = false;

        static_char* LICENSE_TITLE = "SQLCipher (BSD) in DbSqliteCipher plugin";
        static_char* OPENSSL_TITLE = "OpenSSL (used by DbSqliteCipher plugin) license";
};

#endif // DBSQLITECIPHER_H
