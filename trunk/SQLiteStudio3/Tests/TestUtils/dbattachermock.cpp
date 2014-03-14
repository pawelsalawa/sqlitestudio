#include "dbattachermock.h"

bool DbAttacherMock::attachDatabases(const QString& query)
{
    return true;
}

bool DbAttacherMock::attachDatabases(const QList<SqliteQueryPtr>& queries)
{
    return true;
}

bool DbAttacherMock::attachDatabases(SqliteQueryPtr query)
{
    return true;
}

void DbAttacherMock::detachDatabases()
{
}

BiStrHash DbAttacherMock::getDbNameToAttach() const
{
    return BiStrHash();
}

QString DbAttacherMock::getQuery() const
{
    return QString();
}

DbAttacher* DbAttacherFactoryMock::create(Db* db)
{
    return new DbAttacherMock();
}
