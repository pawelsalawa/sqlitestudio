#include "formatorderby.h"
#include "parser/ast/sqliteorderby.h"
#include "parser/ast/sqliteexpr.h"

FormatOrderBy::FormatOrderBy(SqliteOrderBy* orderBy) :
    orderBy(orderBy)
{
}

void FormatOrderBy::formatInternal()
{
    withStatement(orderBy->expr);
    if (orderBy->order != SqliteSortOrder::null)
        withKeyword(sqliteSortOrder(orderBy->order));
}
