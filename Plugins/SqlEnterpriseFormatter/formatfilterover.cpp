#include "formatfilterover.h"
#include "parser/ast/sqliteexpr.h"

FormatFilterOver::FormatFilterOver(SqliteFilterOver* filterOver) :
    FormatStatement(filterOver), filterOver(filterOver)
{
}

void FormatFilterOver::formatInternal()
{
    if (filterOver->filter)
        withStatement(filterOver->filter);

    if (filterOver->over)
        withStatement(filterOver->over);
}

FormatFilterOverFilter::FormatFilterOverFilter(SqliteFilterOver::Filter* filter) :
    FormatStatement(filter), filter(filter)
{
}

void FormatFilterOverFilter::formatInternal()
{
    withKeyword("FILTER").withParExprLeft().withKeyword("WHERE").withStatement(filter->expr).withParExprRight();
}

FormatFilterOverOver::FormatFilterOverOver(SqliteFilterOver::Over* over) :
    FormatStatement(over), over(over)
{
}

void FormatFilterOverOver::formatInternal()
{
    withKeyword("OVER");

    switch (over->mode)
    {
        case SqliteFilterOver::Over::Mode::WINDOW:
            withParExprLeft().withStatement(over->window).withParExprRight();
            break;
        case SqliteFilterOver::Over::Mode::NAME:
            withId(over->name);
            break;
    }
}
