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
    static const QStringList ciphers = {"aes-128-cbc", "aes-128-cfb", "aes-128-cfb1", "aes-128-cfb8", "aes-128-ctr", "aes-128-ecb", "aes-128-gcm", "aes-128-ofb",
                                        "aes-128-xts", "aes-192-cbc", "aes-192-cfb", "aes-192-cfb1", "aes-192-cfb8", "aes-192-ctr", "aes-192-ecb", "aes-192-gcm",
                                        "aes-192-ofb", "aes-256-cbc", "aes-256-cfb", "aes-256-cfb1", "aes-256-cfb8", "aes-256-ctr", "aes-256-ecb", "aes-256-gcm",
                                        "aes-256-ofb", "aes-256-xts", "aes128", "aes192", "aes256", "bf", "bf-cbc", "bf-cfb", "bf-ecb", "bf-ofb", "blowfish",
                                        "camellia-128-cbc", "camellia-128-cfb", "camellia-128-cfb1", "camellia-128-cfb8", "camellia-128-ecb", "camellia-128-ofb",
                                        "camellia-192-cbc", "camellia-192-cfb", "camellia-192-cfb1", "camellia-192-cfb8", "camellia-192-ecb", "camellia-192-ofb",
                                        "camellia-256-cbc", "camellia-256-cfb", "camellia-256-cfb1", "camellia-256-cfb8", "camellia-256-ecb", "camellia-256-ofb",
                                        "camellia128", "camellia192", "camellia256", "cast", "cast-cbc", "cast5-cbc", "cast5-cfb", "cast5-ecb", "cast5-ofb", "des",
                                        "des-cbc", "des-cfb", "des-cfb1", "des-cfb8", "des-ecb", "des-ede", "des-ede-cbc", "des-ede-cfb", "des-ede-ofb", "des-ede3",
                                        "des-ede3-cbc", "des-ede3-cfb", "des-ede3-cfb1", "des-ede3-cfb8", "des-ede3-ofb", "des-ofb", "des3", "desx", "desx-cbc",
                                        "id-aes128-GCM", "id-aes192-GCM", "id-aes256-GCM", "rc2", "rc2-40-cbc", "rc2-64-cbc", "rc2-cbc", "rc2-cfb", "rc2-ecb",
                                        "rc2-ofb", "rc4", "rc4-40", "rc4-hmac-md5", "seed", "seed-cbc", "seed-cfb", "seed-ecb", "seed-ofb"};

    QList<DbPluginOption> opts;

    DbPluginOption opt;
    opt.type = DbPluginOption::PASSWORD;
    opt.key = PASSWORD_OPT;
    opt.label = tr("Password (key)");
    opt.toolTip = tr("Leave empty to create or connect to decrypted database.");
    opt.placeholderText = tr("Encryption password");
    opts << opt;

    opt.type = DbPluginOption::CHOICE;
    opt.key = CIPHER_OPT;
    opt.label = tr("Cipher");
    opt.toolTip = tr("Must be the same as the one used when creating the database. %1 is the default one.").arg(DEF_CIPHER);
    opt.choiceValues = ciphers;
    opt.defaultValue = DEF_CIPHER;
    opt.choiceReadOnly = true;
    opts << opt;

    opt.type = DbPluginOption::INT;
    opt.key = KDF_ITER_OPT;
    opt.label = tr("KDF iterations");
    opt.defaultValue = DEF_KDF_ITER;
    opt.minValue = 0;
    opt.maxValue = std::numeric_limits<int>::max();
    opt.toolTip = tr("Must be the same as the one used when creating the database. %1 is the default.").arg(QString::number(DEF_KDF_ITER));
    opts << opt;

    opt.type = DbPluginOption::INT;
    opt.key = CIPHER_PAGE_SIZE_OPT;
    opt.label = tr("Cipher page size");
    opt.defaultValue = DEF_CIPHER_PAGE_SIZE;
    opt.minValue = 0;
    opt.maxValue = std::numeric_limits<int>::max();
    opt.toolTip = tr("Must be the same as the one used when creating the database. %1 is the default.").arg(QString::number(DEF_CIPHER_PAGE_SIZE));
    opts << opt;

    opt.type = DbPluginOption::BOOL;
    opt.key = CIPHER_1_1_OPT;
    opt.label = tr("1.1 compatibility");
    opt.defaultValue = false;
    opt.toolTip = tr("Enabling this option disables HMAC checks introduced in SQLCipher 2.0, thus making the connection compatible with SQLCipher 1.1.x.");
    opts << opt;

    return opts;
}

bool DbSqliteCipher::init()
{
    Q_INIT_RESOURCE(dbsqlitecipher);

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
    Q_CLEANUP_RESOURCE(dbsqlitecipher);
}

Db *DbSqliteCipher::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqliteCipherInstance(name, path, options);
}
