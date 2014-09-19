#include "sqlitedetach.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"

#include <parser/statementtokenbuilder.h>

SqliteDetach::SqliteDetach()
{
    queryType = SqliteQueryType::Detach;
}

SqliteDetach::SqliteDetach(bool databaseKw, SqliteExpr *name)
    :SqliteDetach()
{
    this->databaseKw = databaseKw;
    this->name = name;
    if (name)
        name->setParent(this);
}

SqliteDetach::~SqliteDetach()
{
}

TokenList SqliteDetach::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("DETACH").withSpace();

    if (databaseKw)
        builder.withKeyword("DATABASE").withSpace();

    builder.withStatement(name).withOperator(";");

    return builder.build();
}
