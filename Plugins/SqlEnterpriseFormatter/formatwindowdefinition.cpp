#include "formatwindowdefinition.h"
#include "parser/ast/sqliteexpr.h"
#include "parser/ast/sqliteorderby.h"

FormatWindowDefinition::FormatWindowDefinition(SqliteWindowDefinition* windowDef) :
    FormatStatement(windowDef), windowDef(windowDef)
{
}

void FormatWindowDefinition::formatInternal()
{
    withId(windowDef->name).withKeyword("AS").withParExprLeft().withStatement(windowDef->window).withParExprRight();
}

FormatWindowDefinitionWindow::FormatWindowDefinitionWindow(SqliteWindowDefinition::Window* window) :
    FormatStatement(window), window(window)
{
}

void FormatWindowDefinitionWindow::formatInternal()
{
    if (!window->name.isNull())
        withId(window->name);

    switch (window->mode)
    {
        case SqliteWindowDefinition::Window::Mode::PARTITION_BY:
            withKeyword("PARTITION").withKeyword("BY").withStatementList(window->exprList);
            break;
        case SqliteWindowDefinition::Window::Mode::ORDER_BY:
            break;
        case SqliteWindowDefinition::Window::Mode::null:
            break;
    }

    if (window->orderBy.size() > 0)
        withKeyword("ORDER").withKeyword("BY").withStatementList(window->orderBy);

    if (window->frame)
        withStatement(window->frame);
}

FormatWindowDefinitionWindowFrame::FormatWindowDefinitionWindowFrame(SqliteWindowDefinition::Window::Frame* frame) :
    FormatStatement(frame), frame(frame)
{
}

void FormatWindowDefinitionWindowFrame::formatInternal()
{
    if (frame->rangeOrRows != SqliteWindowDefinition::Window::Frame::RangeOrRows::null)
        withKeyword(SqliteWindowDefinition::Window::Frame::fromRangeOrRows(frame->rangeOrRows));

    if (frame->endBound)
        withKeyword("BETWEEN").withStatement(frame->startBound).withKeyword("AND").withStatement(frame->endBound);
    else
        withStatement(frame->startBound);

    if (frame->exclude != SqliteWindowDefinition::Window::Frame::Exclude::null)
    {
        withKeyword("EXCLUDE");
        for (const QString& kw : SqliteWindowDefinition::Window::Frame::fromExclude(frame->exclude).split(" "))
            withKeyword(kw);
    }
}

FormatWindowDefinitionWindowFrameBound::FormatWindowDefinitionWindowFrameBound(SqliteWindowDefinition::Window::Frame::Bound* bound) :
    FormatStatement(bound), bound(bound)
{
}

void FormatWindowDefinitionWindowFrameBound::formatInternal()
{
    switch (bound->type)
    {
        case SqliteWindowDefinition::Window::Frame::Bound::Type::UNBOUNDED_PRECEDING:
            withKeyword("UNBOUNDED").withKeyword("PRECEDING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::UNBOUNDED_FOLLOWING:
            withKeyword("UNBOUNDED").withKeyword("FOLLOWING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::EXPR_PRECEDING:
            withStatement(bound->expr).withKeyword("PRECEDING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::EXPR_FOLLOWING:
            withStatement(bound->expr).withKeyword("FOLLOWING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::CURRENT_ROW:
            withKeyword("CURRENT").withKeyword("ROW");
            break;
    }
}
