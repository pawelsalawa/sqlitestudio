#include "formatorderby.h"
#include "parser/ast/sqliteorderby.h"
#include "parser/ast/sqliteexpr.h"

FormatOrderBy::FormatOrderBy(SqliteOrderBy* orderBy) :
    FormatStatement(orderBy), orderBy(orderBy)
{
}

void FormatOrderBy::formatInternal()
{
    withStatement(orderBy->expr);
    if (orderBy->order != SqliteSortOrder::null)
        withKeyword(sqliteSortOrder(orderBy->order));

    if (orderBy->nulls != SqliteNulls::null)
        withKeyword("NULLS").withKeyword(sqliteNulls(orderBy->nulls));
}
