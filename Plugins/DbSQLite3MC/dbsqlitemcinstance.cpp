#include "dbsqlitemcinstance.h"
#include "dbsqlitemc.h"

DbSqliteMcInstance::DbSqliteMcInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3<SQLite3MC>(name, path, connOptions)
{
}

Db* DbSqliteMcInstance::clone() const
{
    return new DbSqliteMcInstance(name, path, connOptions);
}

QString DbSqliteMcInstance::getTypeClassName() const
{
    return "DbSqliteMcInstance";
}

QString DbSqliteMcInstance::getTypeLabel() const
{
    BiStrHash ciphers = DbSqliteMc::getCiphers();
    return ciphers.valueByRight(connOptions[DbSqliteMc::CIPHER_OPT].toString(), AbstractDb3::getTypeLabel());
}

void DbSqliteMcInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString cipher = connOptions[DbSqliteMc::CIPHER_OPT].toString();
    if (!cipher.isEmpty())
    {
        res = exec(QString("PRAGMA cipher = '%1';").arg(cipher), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLite3MC cipher:" << res->getErrorText();
    }

    QString pragmas = connOptions[DbSqliteMc::PRAGMAS_OPT].toString();
    QStringList pragmaList = quickSplitQueries(pragmas);
    for (const QString& pragma : pragmaList)
    {
        QString pragmaSql = removeComments(pragma).trimmed();
        if (pragmaSql.isEmpty())
            continue;

        res = exec(pragmaSql, Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLite3MC pragma" << pragma << ":" << res->getErrorText();
    }

    QString key = connOptions[DbSqliteMc::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(escapeString(key)), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLite3MC key:" << res->getErrorText();
    }

    AbstractDb3<SQLite3MC>::initAfterOpen();
}

QString DbSqliteMcInstance::getAttachSql(Db *otherDb, const QString &generatedAttachName)
{
    QString pass = "";
    if (otherDb->getConnectionOptions().contains(DbSqliteMc::PASSWORD_OPT))
        pass = otherDb->getConnectionOptions()[DbSqliteMc::PASSWORD_OPT].toString().replace("'", "''");

    return QString("ATTACH '%1' AS %2 KEY '%3';").arg(otherDb->getPath(), generatedAttachName, pass);
}
