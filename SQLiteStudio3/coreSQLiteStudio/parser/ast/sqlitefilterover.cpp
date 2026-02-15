#include "sqlitefilterover.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"

SqliteFilterOver::SqliteFilterOver()
{

}

SqliteFilterOver::~SqliteFilterOver()
{
}

SqliteFilterOver::SqliteFilterOver(const SqliteFilterOver& other) :
    SqliteStatement(other)
{
    DEEP_COPY_FIELD(Filter, filter);
    DEEP_COPY_FIELD(Over, over);
}

SqliteFilterOver::SqliteFilterOver(SqliteFilterOver::Filter* filter, SqliteFilterOver::Over* over)
{
    this->filter = filter;
    this->over = over;

    if (filter)
        filter->setParent(this);

    if (over)
        over->setParent(this);
}

SqliteStatement* SqliteFilterOver::clone()
{
    return new SqliteFilterOver(*this);
}

TokenList SqliteFilterOver::rebuildTokensFromContents(bool replaceStatementTokens) const
{
    StatementTokenBuilder builder(replaceStatementTokens);

    if (filter)
        builder.withStatement(filter);

    if (filter || over)
        builder.withSpace();

    if (over)
        builder.withStatement(over);

    return builder.build();
}

SqliteFilterOver::Over::Over()
{
}

SqliteFilterOver::Over::Over(const SqliteFilterOver::Over& other) :
    SqliteStatement(other), name(other.name), mode(other.mode)
{
    DEEP_COPY_FIELD(SqliteWindowDefinition::Window, window);
}

SqliteFilterOver::Over::~Over()
{
}

SqliteFilterOver::Over::Over(SqliteWindowDefinition::Window* window)
{
    this->mode = Mode::WINDOW;
    this->window = window;
    if (window)
        window->setParent(this);
}

SqliteFilterOver::Over::Over(const QString& name)
{
    this->mode = Mode::NAME;
    this->name = name;
}

SqliteStatement* SqliteFilterOver::Over::clone()
{
    return new SqliteFilterOver::Over(*this);
}

TokenList SqliteFilterOver::Over::rebuildTokensFromContents(bool replaceStatementTokens) const
{
    StatementTokenBuilder builder(replaceStatementTokens);

    builder.withKeyword("OVER").withSpace();

    switch (mode)
    {
        case SqliteFilterOver::Over::Mode::WINDOW:
            builder.withParLeft().withStatement(window).withParRight();
            break;
        case SqliteFilterOver::Over::Mode::NAME:
            builder.withOther(name);
            break;
    }

    return builder.build();
}


SqliteFilterOver::Filter::Filter(SqliteExpr* expr)
{
    this->expr = expr;
    if (expr)
        expr->setParent(this);
}

SqliteFilterOver::Filter::Filter(const SqliteFilterOver::Filter& other) :
    SqliteStatement(other)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteStatement* SqliteFilterOver::Filter::clone()
{
    return new SqliteFilterOver::Filter(*this);
}

TokenList SqliteFilterOver::Filter::rebuildTokensFromContents(bool replaceStatementTokens) const
{
    StatementTokenBuilder builder(replaceStatementTokens);

    builder.withKeyword("FILTER").withSpace().withParLeft().withKeyword("WHERE").withSpace().withStatement(expr).withParRight();

    return builder.build();
}


SqliteFilterOver::Filter::~Filter()
{
}
