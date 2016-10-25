#include "dbsqlitewxinstance.h"
#include "dbsqlitewx.h"

DbSqliteWxInstance::DbSqliteWxInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3<WxSQLite>(name, path, connOptions)
{
}

void DbSqliteWxInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString key = connOptions[DbSqliteWx::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(key), Flag::NO_LOCK);
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
