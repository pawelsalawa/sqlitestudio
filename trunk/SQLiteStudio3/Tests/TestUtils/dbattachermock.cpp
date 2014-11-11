#include "dbattachermock.h"

bool DbAttacherMock::attachDatabases(const QString&)
{
    return true;
}

bool DbAttacherMock::attachDatabases(const QList<SqliteQueryPtr>&)
{
    return true;
}

bool DbAttacherMock::attachDatabases(SqliteQueryPtr)
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

DbAttacher* DbAttacherFactoryMock::create(Db*)
{
    return new DbAttacherMock();
}
