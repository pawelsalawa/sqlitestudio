#include "sqliteemptyquery.h"
#include "sqlitequerytype.h"
#include "parser/statementtokenbuilder.h"

SqliteEmptyQuery::SqliteEmptyQuery()
{
    queryType = SqliteQueryType::EMPTY;
}

SqliteEmptyQuery::SqliteEmptyQuery(const SqliteEmptyQuery& other) :
    SqliteEmptyQuery()
{
}

TokenList SqliteEmptyQuery::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withOperator(";");
    return builder.build();
}
