#include "sqliterelease.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteRelease::SqliteRelease()
{
    queryType = SqliteQueryType::Release;
}

SqliteRelease::SqliteRelease(const SqliteRelease& other) :
    SqliteQuery(other), name(other.name), savepointKw(other.savepointKw)
{
}

SqliteRelease::SqliteRelease(bool savepointKw, const QString& name)
    : SqliteRelease()
{
    this->name = name;
    this->savepointKw = savepointKw;
}

SqliteStatement*SqliteRelease::clone()
{
    return new SqliteRelease(*this);
}

TokenList SqliteRelease::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("RELEASE").withSpace();
    if (savepointKw)
        builder.withKeyword("SAVEPOINT").withSpace();

    builder.withOther(name, dialect).withOperator(";");

    return builder.build();
}
