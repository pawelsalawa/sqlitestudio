#include "sqliterelease.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

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

TokenList SqliteRelease::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("RELEASE").withSpace();
    if (savepointKw)
        builder.withKeyword("SAVEPOINT").withSpace();

    builder.withOther(name, dialect).withOperator(";");

    return builder.build();
}
