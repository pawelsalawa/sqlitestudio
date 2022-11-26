#include "dbsqlitecipher.h"
#include "sqlitestudio.h"
#include "services/extralicensemanager.h"
#include "common/unused.h"
#include "dbsqlitecipherinstance.h"
#include "services/notifymanager.h"
#include <limits>

DbSqliteCipher::DbSqliteCipher()
{
}

QString DbSqliteCipher::getLabel() const
{
    return "SQLCipher";
}

bool DbSqliteCipher::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqliteCipherInstance*>(db));
}

QList<DbPluginOption> DbSqliteCipher::getOptionsList() const
{
    QList<DbPluginOption> opts;

    DbPluginOption opt;
    opt.type = DbPluginOption::PASSWORD;
    opt.key = PASSWORD_OPT;
    opt.label = tr("Password (key)");
    opt.toolTip = tr("Leave empty to create or connect to decrypted database.");
    opt.placeholderText = tr("Encryption password");
    opts << opt;

    opt.type = DbPluginOption::SQL;
    opt.key = PRAGMAS_OPT;
    opt.label = tr("Cipher configuration (optional)");
    opt.toolTip = tr("PRAGMA statements to customize SQLCipher configuration, such as KDF iterations, legacy mode, etc.\n"
                            "They will be executed upon each opening of the database.\n"
                            "See documentation for SQLCipher for details.");
    opts << opt;

    return opts;
}

bool DbSqliteCipher::init()
{
    SQLS_INIT_RESOURCE(dbsqlitecipher);

    if (!SQLITESTUDIO->getExtraLicenseManager()->addLicense(LICENSE_TITLE, ":/license/sqlcipher.txt"))
    {
        qCritical() << "Could not register SQLCipher license.";
        return false;
    }

    if (!SQLITESTUDIO->getExtraLicenseManager()->addLicense(OPENSSL_TITLE, ":/license/openssl_lic.txt"))
    {
        qCritical() << "Could not register OpenSSL license.";
        return false;
    }

    initValid = true;
    return true;
}

void DbSqliteCipher::deinit()
{
    SQLITESTUDIO->getExtraLicenseManager()->removeLicense(LICENSE_TITLE);
    SQLITESTUDIO->getExtraLicenseManager()->removeLicense(OPENSSL_TITLE);
    SQLS_CLEANUP_RESOURCE(dbsqlitecipher);
}

Db *DbSqliteCipher::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqliteCipherInstance(name, path, options);
}
