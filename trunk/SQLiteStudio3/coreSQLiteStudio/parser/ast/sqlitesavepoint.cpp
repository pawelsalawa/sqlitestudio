#include "sqlitesavepoint.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteSavepoint::SqliteSavepoint()
{
    queryType = SqliteQueryType::Savepoint;
}

SqliteSavepoint::SqliteSavepoint(const SqliteSavepoint& other) :
    SqliteQuery(other), name(other.name)
{
}

SqliteSavepoint::SqliteSavepoint(const QString &name)
    : SqliteSavepoint()
{
    this->name = name;
}

SqliteStatement*SqliteSavepoint::clone()
{
    return new SqliteSavepoint(*this);
}

TokenList SqliteSavepoint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("SAVEPOINT").withSpace().withOther(name, dialect).withOperator(";");
    return builder.build();
}
