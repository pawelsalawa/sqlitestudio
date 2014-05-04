#include "sqlitelimit.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"

SqliteLimit::SqliteLimit()
{
}

SqliteLimit::SqliteLimit(const SqliteLimit& other) :
    SqliteStatement(other)
{
    DEEP_COPY_FIELD(SqliteExpr, limit);
    DEEP_COPY_FIELD(SqliteExpr, offset);
}

SqliteLimit::SqliteLimit(SqliteExpr *expr)
{
    limit = expr;
    if (expr)
        expr->setParent(this);
}

SqliteLimit::SqliteLimit(SqliteExpr *expr1, SqliteExpr *expr2, bool offsetKeyword)
{
    limit = expr1;
    offset = expr2;
    offsetKw = offsetKeyword;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);
}

SqliteLimit::SqliteLimit(const QVariant &positiveInt)
{
    limit = new SqliteExpr();
    limit->initLiteral(positiveInt);
    limit->setParent(this);
}

SqliteLimit::SqliteLimit(const QVariant &positiveInt1, const QVariant &positiveInt2)
{
    limit = new SqliteExpr();
    limit->initLiteral(positiveInt1);
    limit->setParent(this);

    offset = new SqliteExpr();
    offset->initLiteral(positiveInt2);
    offset->setParent(this);
}

SqliteLimit::~SqliteLimit()
{
}


TokenList SqliteLimit::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withKeyword("LIMIT").withStatement(limit);
    if (offset)
    {
        if (offsetKw)
            builder.withSpace().withKeyword("OFFSET");
        else
            builder.withOperator(",");

        builder.withStatement(offset);
    }

    return builder.build();
}
