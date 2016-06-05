#include "dbsqlitecipherinstance.h"
#include "dbsqlitecipher.h"
#include <QDebug>

DbSqliteCipherInstance::DbSqliteCipherInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3<SqlCipher>(name, path, connOptions)
{
}

void DbSqliteCipherInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString key = connOptions[DbSqliteCipher::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(key), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLCipher key:" << res->getErrorText();
    }

    QString cipher = connOptions[DbSqliteCipher::CIPHER_OPT].toString();
    if (!cipher.isEmpty() && cipher != DbSqliteCipher::DEF_CIPHER)
    {
        res = exec(QString("PRAGMA cipher = '%1';").arg(cipher), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLCipher cipher:" << res->getErrorText();
    }

    if (connOptions.contains(DbSqliteCipher::KDF_ITER_OPT))
    {
        int kdfIter = connOptions[DbSqliteCipher::KDF_ITER_OPT].toInt();
        if (kdfIter >= 0 && kdfIter != DbSqliteCipher::DEF_KDF_ITER)
        {
            res = exec(QString("PRAGMA kdf_iter = '%1';").arg(kdfIter), Flag::NO_LOCK);
            if (res->isError())
                qWarning() << "Error while defining SQLCipher kdf_iter:" << res->getErrorText();
        }
    }

    if (connOptions.contains(DbSqliteCipher::CIPHER_PAGE_SIZE_OPT))
    {
        int pageSize = connOptions[DbSqliteCipher::CIPHER_PAGE_SIZE_OPT].toInt();
        if (pageSize >= 0 && pageSize != DbSqliteCipher::DEF_CIPHER_PAGE_SIZE)
        {
            res = exec(QString("PRAGMA cipher_page_size = %1;").arg(pageSize), Flag::NO_LOCK);
            if (res->isError())
                qWarning() << "Error while defining SQLCipher cipher_page_size:" << res->getErrorText();
        }
    }

    bool hmacCompatibility = connOptions[DbSqliteCipher::CIPHER_1_1_OPT].toBool();
    if (hmacCompatibility)
    {
        res = exec(QString("PRAGMA cipher_use_hmac = OFF;"), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining cipher_use_hmac:" << res->getErrorText();
    }

    AbstractDb3<SqlCipher>::initAfterOpen();
}

QString DbSqliteCipherInstance::getAttachSql(Db* otherDb, const QString& generatedAttachName)
{
    QString pass = "";
    if (otherDb->getConnectionOptions().contains(DbSqliteCipher::PASSWORD_OPT))
        pass = otherDb->getConnectionOptions()[DbSqliteCipher::PASSWORD_OPT].toString().replace("'", "''");

    return QString("ATTACH '%1' AS %2 KEY '%3';").arg(otherDb->getPath(), generatedAttachName, pass);
}
