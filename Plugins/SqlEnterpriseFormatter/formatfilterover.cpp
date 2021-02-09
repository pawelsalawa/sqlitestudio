#include "formatfilterover.h"
#include "parser/ast/sqliteexpr.h"

FormatFilterOver::FormatFilterOver(SqliteFilterOver* filterOver) :
    filterOver(filterOver)
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
    filter(filter)
{
}

void FormatFilterOverFilter::formatInternal()
{
    withKeyword("FILTER").withParFuncLeft().withKeyword("WHERE").withStatement(filter->expr).withParFuncRight();
}

FormatFilterOverOver::FormatFilterOverOver(SqliteFilterOver::Over* over) :
    over(over)
{
}

void FormatFilterOverOver::formatInternal()
{
    withKeyword("OVER");

    switch (over->mode)
    {
        case SqliteFilterOver::Over::Mode::WINDOW:
            withParFuncLeft().withStatement(over->window).withParFuncRight();
            break;
        case SqliteFilterOver::Over::Mode::NAME:
            withId(over->name);
            break;
    }
}
