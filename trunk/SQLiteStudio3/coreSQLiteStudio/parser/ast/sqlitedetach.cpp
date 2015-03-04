#include "sqlitedetach.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "common/global.h"
#include "parser/statementtokenbuilder.h"

SqliteDetach::SqliteDetach()
{
    queryType = SqliteQueryType::Detach;
}

SqliteDetach::SqliteDetach(const SqliteDetach& other) :
    SqliteQuery(other), databaseKw(other.databaseKw)
{
    DEEP_COPY_FIELD(SqliteExpr, name);
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

SqliteStatement*SqliteDetach::clone()
{
    return new SqliteDetach(*this);
}

TokenList SqliteDetach::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("DETACH").withSpace();

    if (databaseKw)
        builder.withKeyword("DATABASE").withSpace();

    builder.withStatement(name).withOperator(";");

    return builder.build();
}
