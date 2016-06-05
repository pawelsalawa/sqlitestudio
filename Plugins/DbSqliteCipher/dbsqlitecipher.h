#ifndef DBSQLITECIPHER_H
#define DBSQLITECIPHER_H

#include "dbsqlitecipher_global.h"
#include "plugins/dbplugin.h"
#include "plugins/genericplugin.h"

class DBSQLITECIPHERSHARED_EXPORT DbSqliteCipher : public GenericPlugin, public DbPlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbsqlitecipher.json")

    public:
        DbSqliteCipher();

        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        Db* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage);
        QList<DbPluginOption> getOptionsList() const;
        QString generateDbName(const QVariant& baseValue);
        bool init();
        void deinit();

        static_char* PASSWORD_OPT = "password";
        static_char* CIPHER_OPT = "cipher";
        static_char* KDF_ITER_OPT = "kdf_iter";
        static_char* CIPHER_PAGE_SIZE_OPT = "cipher_page_size";
        static_char* CIPHER_1_1_OPT = "cipher_1.1_compatibility";
        static const int DEF_CIPHER_PAGE_SIZE = 1024;
        static const int DEF_KDF_ITER = 64000;
        static_char* DEF_CIPHER = "aes-256-cbc";

    private:
        bool initValid = false;

        static_char* LICENSE_TITLE = "SQLCipher (BSD) in DbSqliteCipher plugin";
};

#endif // DBSQLITECIPHER_H
