#include "dbsqlitecipherinstance.h"
#include "dbsqlitecipher.h"
#include <QDebug>

DbSqliteCipherInstance::DbSqliteCipherInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3<SqlCipher>(name, path, connOptions)
{
}

Db* DbSqliteCipherInstance::clone() const
{
    return new DbSqliteCipherInstance(name, path, connOptions);
}

QString DbSqliteCipherInstance::getTypeClassName() const
{
    return "DbSqliteCipherInstance";
}

void DbSqliteCipherInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString key = connOptions[DbSqliteCipher::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(escapeString(key)), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLCipher key:" << res->getErrorText();
    }

    QString pragmas = connOptions[DbSqliteCipher::PRAGMAS_OPT].toString();
    QStringList pragmaList = quickSplitQueries(pragmas);
    for (const QString& pragma : pragmaList)
    {
        res = exec(pragma, Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLCipher pragma" << pragma << ":" << res->getErrorText();
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
