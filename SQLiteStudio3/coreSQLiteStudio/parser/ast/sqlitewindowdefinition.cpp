#include "sqlitewindowdefinition.h"
#include "sqliteexpr.h"
#include "sqliteorderby.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include <QDebug>

SqliteWindowDefinition::SqliteWindowDefinition()
{

}

SqliteWindowDefinition::SqliteWindowDefinition(const SqliteWindowDefinition& other) :
    SqliteStatement(other), name(other.name)
{
    DEEP_COPY_FIELD(Window, window);
}

SqliteWindowDefinition::SqliteWindowDefinition(const QString& name, SqliteWindowDefinition::Window* window)
{
    this->name = name;
    this->window = window;
    if (window)
        window->setParent(this);
}

SqliteStatement* SqliteWindowDefinition::clone()
{
    return new SqliteWindowDefinition(*this);
}

TokenList SqliteWindowDefinition::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withOther(name).withSpace().withKeyword("AS").withParLeft().withStatement(window).withParRight();

    return builder.build();
}

SqliteWindowDefinition::Window::Window()
{
}

SqliteWindowDefinition::Window::Window(const SqliteWindowDefinition::Window& other) :
    SqliteStatement(other), name(other.name), mode(other.mode)
{
    DEEP_COPY_COLLECTION(SqliteExpr, exprList);
    DEEP_COPY_COLLECTION(SqliteOrderBy, orderBy);
    DEEP_COPY_FIELD(Frame, frame);
}

SqliteStatement* SqliteWindowDefinition::Window::clone()
{
    return new Window(*this);
}

void SqliteWindowDefinition::Window::initPartitionBy(const QString& name, const QList<SqliteExpr*>& exprList, const QList<SqliteOrderBy*>& orderBy, SqliteWindowDefinition::Window::Frame* frame)
{
    this->mode = Mode::PARTITION_BY;
    this->name = name;
    initExprList(exprList);
    initOrderBy(orderBy);
    initFrame(frame);
}

void SqliteWindowDefinition::Window::initOrderBy(const QString& name, const QList<SqliteOrderBy*>& orderBy, SqliteWindowDefinition::Window::Frame* frame)
{
    this->mode = Mode::ORDER_BY;
    this->name = name;
    initOrderBy(orderBy);
    initFrame(frame);
}

void SqliteWindowDefinition::Window::init(const QString& name, SqliteWindowDefinition::Window::Frame* frame)
{
    this->mode = Mode::null;
    this->name = name;
    initFrame(frame);
}

TokenList SqliteWindowDefinition::Window::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (!name.isNull())
        builder.withOther(name).withSpace();

    switch (mode)
    {
        case SqliteWindowDefinition::Window::Mode::PARTITION_BY:
            builder.withKeyword("PARTITION").withSpace().withKeyword("BY").withSpace().withStatementList(exprList).withSpace();
            break;
        case SqliteWindowDefinition::Window::Mode::ORDER_BY:
            break;
        case SqliteWindowDefinition::Window::Mode::null:
            break;
    }

    if (orderBy.size() > 0)
        builder.withKeyword("ORDER").withSpace().withKeyword("BY").withSpace().withStatementList(orderBy);

    if (frame)
        builder.withStatement(frame);

    return builder.build();
}

void SqliteWindowDefinition::Window::initExprList(const QList<SqliteExpr*>& exprList)
{
    this->exprList = exprList;
    for (SqliteExpr* expr : exprList)
        expr->setParent(this);
}

void SqliteWindowDefinition::Window::initOrderBy(const QList<SqliteOrderBy*>& orderBy)
{
    this->orderBy = orderBy;
    for (SqliteOrderBy* order : orderBy)
        order->setParent(this);
}

void SqliteWindowDefinition::Window::initFrame(SqliteWindowDefinition::Window::Frame* frame)
{
    this->frame = frame;
    if (frame)
        frame->setParent(this);
}

SqliteWindowDefinition::Window::Frame::RangeOrRows SqliteWindowDefinition::Window::Frame::toRangeOrRows(const QString& value)
{
    QString upper = value.toUpper();
    if (upper == "RANGE")
        return RangeOrRows::RANGE;
    else if (upper == "ROWS")
        return RangeOrRows::ROWS;
    else if (upper == "GROUPS")
        return RangeOrRows::GROUPS;
    else
        return RangeOrRows::null;
}

QString SqliteWindowDefinition::Window::Frame::fromRangeOrRows(SqliteWindowDefinition::Window::Frame::RangeOrRows value)
{
    switch (value)
    {
        case SqliteWindowDefinition::Window::Frame::RangeOrRows::RANGE:
            return "RANGE";
        case SqliteWindowDefinition::Window::Frame::RangeOrRows::ROWS:
            return "ROWS";
        case SqliteWindowDefinition::Window::Frame::RangeOrRows::GROUPS:
            return "GROUPS";
        case SqliteWindowDefinition::Window::Frame::RangeOrRows::null:
            break;
    }
    return QString();
}

SqliteWindowDefinition::Window::Frame::Exclude SqliteWindowDefinition::Window::Frame::toExclude(const QString& value)
{
    QString upper = value.toUpper();
    if (upper == "NO OTHERS")
        return Exclude::NO_OTHERS;
    else if (upper == "CURRENT ROW")
        return Exclude::CURRENT_ROW;
    else if (upper == "GROUP")
        return Exclude::GROUP;
    else if (upper == "TIES")
        return Exclude::TIES;
    else
        return Exclude::null;
}

QString SqliteWindowDefinition::Window::Frame::fromExclude(SqliteWindowDefinition::Window::Frame::Exclude value)
{
    switch (value)
    {
        case SqliteWindowDefinition::Window::Frame::Exclude::TIES:
            return "TIES";
        case SqliteWindowDefinition::Window::Frame::Exclude::NO_OTHERS:
            return "NO OTHERS";
        case SqliteWindowDefinition::Window::Frame::Exclude::CURRENT_ROW:
            return "CURRENT ROW";
        case SqliteWindowDefinition::Window::Frame::Exclude::GROUP:
            return "GROUP";
        case SqliteWindowDefinition::Window::Frame::Exclude::null:
            break;
    }
    return QString();
}

SqliteWindowDefinition::Window::Frame::Frame()
{
}

SqliteWindowDefinition::Window::Frame::Frame(const SqliteWindowDefinition::Window::Frame& other) :
    SqliteStatement(other), rangeOrRows(other.rangeOrRows), exclude(other.exclude)
{
    DEEP_COPY_FIELD(Bound, startBound);
    DEEP_COPY_FIELD(Bound, endBound);
}

SqliteWindowDefinition::Window::Frame::Frame(SqliteWindowDefinition::Window::Frame::RangeOrRows rangeOrRows,
                                             SqliteWindowDefinition::Window::Frame::Bound* startBound,
                                             SqliteWindowDefinition::Window::Frame::Bound* endBound,
                                             SqliteWindowDefinition::Window::Frame::Exclude exclude)
{
    this->rangeOrRows = rangeOrRows;
    this->startBound = startBound;
    this->endBound = endBound;
    this->exclude = exclude;

    if (startBound)
        startBound->setParent(this);

    if (endBound)
        endBound->setParent(this);
}

SqliteStatement* SqliteWindowDefinition::Window::Frame::clone()
{
    return new Frame(*this);
}

TokenList SqliteWindowDefinition::Window::Frame::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (rangeOrRows != RangeOrRows::null)
        builder.withKeyword(fromRangeOrRows(rangeOrRows)).withSpace();

    if (endBound)
        builder.withKeyword("BETWEEN").withSpace().withStatement(startBound).withSpace()
                .withKeyword("AND").withSpace().withStatement(endBound);
    else
        builder.withStatement(startBound);

    if (exclude != Exclude::null)
    {
        builder.withSpace().withKeyword("EXCLUDE");
        for (const QString& kw : fromExclude(exclude).split(" "))
            builder.withSpace().withKeyword(kw);
    }

    return builder.build();
}

SqliteWindowDefinition::Window::Frame::Bound::Bound()
{
}

SqliteWindowDefinition::Window::Frame::Bound::Bound(const SqliteWindowDefinition::Window::Frame::Bound& other) :
    SqliteStatement(other), type(other.type)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteWindowDefinition::Window::Frame::Bound::Bound(SqliteExpr* expr, const QString& value)
{
    this->expr = expr;
    if (expr)
        expr->setParent(this);

    QString upper = value.toUpper();
    if (upper == "UNBOUNDED PRECEDING")
        type = Type::UNBOUNDED_PRECEDING;
    else if (expr && upper == "PRECEDING")
        type = Type::EXPR_PRECEDING;
    else if (upper == "UNBOUNDED FOLLOWING")
        type = Type::UNBOUNDED_FOLLOWING;
    else if (expr && upper == "FOLLOWING")
        type = Type::EXPR_FOLLOWING;
    else if (upper == "CURRENT ROW")
        type = Type::CURRENT_ROW;
    else
        qCritical() << "Unexpected Window Frame Bound:" << value;
}

SqliteStatement* SqliteWindowDefinition::Window::Frame::Bound::clone()
{
    return new Bound(*this);
}

TokenList SqliteWindowDefinition::Window::Frame::Bound::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    switch (type)
    {
        case SqliteWindowDefinition::Window::Frame::Bound::Type::UNBOUNDED_PRECEDING:
            builder.withKeyword("UNBOUNDED").withSpace().withKeyword("PRECEDING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::UNBOUNDED_FOLLOWING:
            builder.withKeyword("UNBOUNDED").withSpace().withKeyword("FOLLOWING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::EXPR_PRECEDING:
            builder.withStatement(expr).withSpace().withKeyword("PRECEDING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::EXPR_FOLLOWING:
            builder.withStatement(expr).withSpace().withKeyword("FOLLOWING");
            break;
        case SqliteWindowDefinition::Window::Frame::Bound::Type::CURRENT_ROW:
            builder.withKeyword("CURRENT").withSpace().withKeyword("ROW");
            break;
    }

    return builder.build();
}
