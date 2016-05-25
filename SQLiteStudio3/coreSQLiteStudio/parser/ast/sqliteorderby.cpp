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

SqliteStatement*SqliteOrderBy::clone()
{
    return new SqliteOrderBy(*this);
}

bool SqliteOrderBy::isSimpleColumn() const
{
    return !getColumnName().isEmpty();
}

QString SqliteOrderBy::getColumnName() const
{
    if (!expr)
        return QString();

    if (expr->mode == SqliteExpr::Mode::ID)
        return expr->column;

    if (expr->mode == SqliteExpr::Mode::COLLATE && expr->expr1 && expr->expr1->mode == SqliteExpr::Mode::ID)
        return expr->expr1->literalValue.toString();

    return QString();
}

QString SqliteOrderBy::getCollation() const
{
    if (expr->mode == SqliteExpr::Mode::COLLATE)
        return expr->collation;

    return QString();
}

QString SqliteOrderBy::getColumnString() const
{
    QString res = getColumnName();
    if (res.isNull())
        return expr->detokenize();

    return res;
}

void SqliteOrderBy::setColumnName(const QString& name)
{
    if (expr && expr->mode == SqliteExpr::Mode::COLLATE)
    {
        safe_delete(expr->expr1);
        expr->expr1 = new SqliteExpr();
        expr->expr1->setParent(expr);
        expr->expr1->initId(name);
    }
    else
    {
        safe_delete(expr);
        expr = new SqliteExpr();
        expr->setParent(this);
        expr->initId(name);
    }
}

void SqliteOrderBy::setCollation(const QString& name)
{
    if (expr && expr->mode == SqliteExpr::Mode::COLLATE)
    {
        expr->collation = name;
    }
    else
    {
        SqliteExpr* theExpr = expr;
        SqliteExpr* collationExpr = new SqliteExpr();
        collationExpr->initCollate(theExpr, name);
        theExpr->setParent(collationExpr);
        collationExpr->setParent(this);
    }
}

TokenList SqliteOrderBy::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr);
    if (order != SqliteSortOrder::null)
        builder.withSpace().withKeyword(sqliteSortOrder(order));

    return builder.build();
}


void SqliteOrderBy::clearCollation()
{
    if (expr->mode != SqliteExpr::Mode::COLLATE)
        return;

    SqliteExpr* tmpExpr = expr;
    expr = tmpExpr->expr1;
    expr->setParent(this);
    delete tmpExpr;
}
