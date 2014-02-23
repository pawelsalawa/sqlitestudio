#include "sqliteemptyquery.h"
#include "sqlitequerytype.h"

SqliteEmptyQuery::SqliteEmptyQuery()
{
    queryType = SqliteQueryType::EMPTY;
}
