#include "dbsqlitesystemdatainstance.h"
#include "dbsqlitesystemdata.h"

DbSqliteSystemDataInstance::DbSqliteSystemDataInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3<SystemDataSQLite>(name, path, connOptions)
{
}

void DbSqliteSystemDataInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString key = connOptions[DbSqliteSystemData::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(key), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining WxSqlite3 key:" << res->getErrorText();
    }

    AbstractDb3<SystemDataSQLite>::initAfterOpen();
}

QString DbSqliteSystemDataInstance::getAttachSql(Db *otherDb, const QString &generatedAttachName)
{
    QString pass = "";
    if (otherDb->getConnectionOptions().contains(DbSqliteSystemData::PASSWORD_OPT))
        pass = otherDb->getConnectionOptions()[DbSqliteSystemData::PASSWORD_OPT].toString().replace("'", "''");

    return QString("ATTACH '%1' AS %2 KEY '%3';").arg(otherDb->getPath(), generatedAttachName, pass);
}
