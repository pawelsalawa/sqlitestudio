#include "sqliterelease.h"
#include "sqlitequerytype.h"

SqliteRelease::SqliteRelease()
{
    queryType = SqliteQueryType::Release;
}

SqliteRelease::SqliteRelease(bool savepointKw, const QString& name)
    : SqliteRelease()
{
    this->name = name;
    this->savepointKw = savepointKw;
}
