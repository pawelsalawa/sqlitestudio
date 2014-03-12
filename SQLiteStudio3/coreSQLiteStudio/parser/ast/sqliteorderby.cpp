#include "sqliteorderby.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"

SqliteOrderBy::SqliteOrderBy()
{
}

SqliteOrderBy::SqliteOrderBy(const SqliteOrderBy& other) :
    SqliteStatement(other), order(other.order)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteOrderBy::SqliteOrderBy(SqliteExpr *expr, SqliteSortOrder order)
{
    this->expr = expr;
    this->order = order;
    if (expr)
        expr->setParent(this);
}

SqliteOrderBy::~SqliteOrderBy()
{
}

TokenList SqliteOrderBy::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr);
    if (order != SqliteSortOrder::null)
        builder.withSpace().withKeyword(sqliteSortOrder(order));

    return builder.build();
}
