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
    Q_UNUSED(other);
}

SqliteStatement*SqliteEmptyQuery::clone()
{
    return new SqliteEmptyQuery(*this);
}

TokenList SqliteEmptyQuery::rebuildTokensFromContents(bool replaceStatementTokens) const
{
    StatementTokenBuilder builder(replaceStatementTokens);
    builder.withOperator(";");
    return builder.build();
}
