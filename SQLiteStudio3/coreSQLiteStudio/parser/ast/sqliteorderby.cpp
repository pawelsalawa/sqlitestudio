#include "sqliteorderby.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include <QDebug>

SqliteOrderBy::SqliteOrderBy()
{
}

SqliteOrderBy::SqliteOrderBy(const SqliteOrderBy& other) :
    SqliteStatement(other), order(other.order), nulls(other.nulls)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteOrderBy::SqliteOrderBy(SqliteExpr *expr, SqliteSortOrder order, SqliteNulls nulls)
{
    this->expr = expr;
    this->order = order;
    this->nulls = nulls;
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
        return expr->expr1->column;

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
    if (!expr)
        return;

    if (expr->mode == SqliteExpr::Mode::COLLATE)
    {
        expr->collation = name;
        return;
    }

    SqliteExpr* collationExpr = new SqliteExpr();
    collationExpr->initCollate(expr, name);
    expr->setParent(collationExpr);
    collationExpr->setParent(this);
    expr = collationExpr;
}

TokenList SqliteOrderBy::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr);
    if (order != SqliteSortOrder::null)
        builder.withSpace().withKeyword(sqliteSortOrder(order));

    if (nulls != SqliteNulls::null)
        builder.withSpace().withKeyword("NULLS").withSpace().withKeyword(sqliteNulls(nulls));

    return builder.build();
}

void SqliteOrderBy::evaluatePostParsing()
{
    pullLastCollationAsOuterExpr();
}

void SqliteOrderBy::pullLastCollationAsOuterExpr()
{
    /*
     * If the order statement is like: columnName + 2 COLLATE BINARY ASC
     * then the COLLATE is associated with the "2" subexpr, instead of the most outer expr.
     * Looks like SQLite's parser does the same, but they don't care about the depth as we do here.
     * Therefore if we idenfity this case, we need to pull the inner expr to outside.
     */
    TokenPtr collateToken = expr->tokens.findLast(Token::KEYWORD, "COLLATE", Qt::CaseInsensitive);
    if (collateToken.isNull())
        return;

    int lastCollateIdx = expr->tokens.indexOf(collateToken);
    if (expr->tokens.mid(lastCollateIdx).filterWhiteSpaces().size() != 2)
        return;

    // This is the case. We need to pull the expr to the top level.
    SqliteStatement* stmt = expr->findStatementWithToken(collateToken);
    SqliteExpr* collateExpr = dynamic_cast<SqliteExpr*>(stmt);
    if (!collateExpr)
    {
        qCritical() << "Could not cast statement to SqliteExpr, even though it's identified as COLLATE expr. The actual contents:"
                    << collateExpr->detokenize();
        return;
    }

    if (collateExpr == expr)
        return; // it's already the top-level expr, we're fine.

    SqliteExpr* parentExpr = dynamic_cast<SqliteExpr*>(collateExpr->parentStatement());
    if (!parentExpr)
    {
        qCritical() << "Could not cast parent statement to SqliteExpr, even though parent of COLLATE should be another expr at this stage."
                    << "The qobject type of parent:" << collateExpr->parentStatement()->metaObject()->className();
        return;
    }

    // Take out COLLATE from its current place
    collateExpr->expr1->setParent(parentExpr);             // New parent of COLLATE's expr is now parent of COLLATE
    parentExpr->replace(collateExpr, collateExpr->expr1);  // New child expr of COLLATE's parent is now child of COLLATE

    // Put it at top level
    collateExpr->expr1 = expr;      // COLLATE's child is set to the old top level expr
    expr->setParent(collateExpr);   // Old top level expr gets COLLATE as parent
    expr = collateExpr;             // New top level is now COLLATE
    collateExpr->setParent(this);   // COLLATE's new parent is this

    rebuildTokens();
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
