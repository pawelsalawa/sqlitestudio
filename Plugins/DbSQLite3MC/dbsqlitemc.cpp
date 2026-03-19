#include "dbsqlitemc.h"
#include "dbsqlitemcinstance.h"
#include <QMap>

DbSqliteMc::DbSqliteMc()
{
}

BiStrHash DbSqliteMc::getCiphers()
{
    static const BiStrHash ciphers = {
        {"wxSQLite3: AES 128 Bit", "aes128cbc"},
        {"wxSQLite3: AES 256 Bit", "aes256cbc"},
        {"sqleet: ChaCha20-Poly1305", "chacha20"},
        {"SQLCipher: AES 256 Bit", "sqlcipher"},
        {"System.Data.SQLite: RC4", "rc4"},
        {"Ascon-128 v1.2", "ascon128"},
        {"AEGIS", "aegis"},
    };
    return ciphers;
}

QString DbSqliteMc::getLabel() const
{
    return "SQLite 3 Multiple Ciphers";
}

QList<DbPluginOption> DbSqliteMc::getOptionsList() const
{
    static_qstring(defaultCipher, "aes256cbc");

    QMap<QString, QVariant> ciphers;
    QHash<QString, QString> cipherHash = getCiphers().toHash();
    for (auto it = cipherHash.constBegin(); it != cipherHash.constEnd(); ++it)
        ciphers.insert(it.key(), QVariant(it.value()));

    QList<DbPluginOption> opts;

    DbPluginOption optPass;
    optPass.type = DbPluginOption::PASSWORD;
    optPass.key = PASSWORD_OPT;
    optPass.label = tr("Password (key)");
    optPass.toolTip = tr("Leave empty to create or connect to decrypted database.");
    optPass.placeholderText = tr("Encryption password");
    opts << optPass;

    DbPluginOption optCipher;
    optCipher.type = DbPluginOption::CHOICE;
    optCipher.key = CIPHER_OPT;
    optCipher.label = tr("Cipher");
    optCipher.toolTip = tr("Cipher determines encryption algorithm used to encrypt the database.");
    optCipher.choiceDataValues = ciphers;
    optCipher.defaultValue = defaultCipher;
    opts << optCipher;

    DbPluginOption optPragmas;
    optPragmas.type = DbPluginOption::SQL;
    optPragmas.key = PRAGMAS_OPT;
    optPragmas.label = tr("Cipher configuration (optional)");
    optPragmas.toolTip = tr("PRAGMA statements to customize SQLite3 Multiple Ciphers configuration, such as KDF iterations, legacy mode, etc.\n"
                            "They will be executed upon each opening of the database.\n"
                            "See documentation for SQLite3 Multiple Ciphers for details.");
    opts << optPragmas;


    return opts;
}

bool DbSqliteMc::checkIfDbServedByPlugin(Db *db) const
{
    return (db && dynamic_cast<DbSqliteMcInstance*>(db));
}

Db *DbSqliteMc::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqliteMcInstance(name, path, options);
}
