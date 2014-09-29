#include "sqliteemptyquery.h"
#include "sqlitequerytype.h"
#include "parser/statementtokenbuilder.h"
#include "common/unused.h"

SqliteEmptyQuery::SqliteEmptyQuery()
{
    queryType = SqliteQueryType::EMPTY;
}

SqliteEmptyQuery::SqliteEmptyQuery(const SqliteEmptyQuery& other) :
    SqliteEmptyQuery()
{
    UNUSED(other);
}

SqliteStatement*SqliteEmptyQuery::clone()
{
    return new SqliteEmptyQuery(*this);
}

TokenList SqliteEmptyQuery::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withOperator(";");
    return builder.build();
}
