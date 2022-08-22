#include "dbsqlitewxinstance.h"
#include "dbsqlitewx.h"

DbSqliteWxInstance::DbSqliteWxInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3<WxSQLite>(name, path, connOptions)
{
}

void DbSqliteWxInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString cipher = connOptions[DbSqliteWx::CIPHER_OPT].toString();
    if (!cipher.isEmpty())
    {
        res = exec(QString("PRAGMA cipher = '%1';").arg(cipher), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining WxSqlite3 cipher:" << res->getErrorText();
    }

    QString pragmas = connOptions[DbSqliteWx::PRAGMAS_OPT].toString();
    QStringList pragmaList = quickSplitQueries(pragmas);
    for (const QString& pragma : pragmaList)
    {
        res = exec(pragma, Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining WxSqlite3 pragma" << pragma << ":" << res->getErrorText();
    }

    QString key = connOptions[DbSqliteWx::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(escapeString(key)), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining WxSqlite3 key:" << res->getErrorText();
    }

    AbstractDb3<WxSQLite>::initAfterOpen();
}

QString DbSqliteWxInstance::getAttachSql(Db *otherDb, const QString &generatedAttachName)
{
    QString pass = "";
    if (otherDb->getConnectionOptions().contains(DbSqliteWx::PASSWORD_OPT))
        pass = otherDb->getConnectionOptions()[DbSqliteWx::PASSWORD_OPT].toString().replace("'", "''");

    return QString("ATTACH '%1' AS %2 KEY '%3';").arg(otherDb->getPath(), generatedAttachName, pass);
}
