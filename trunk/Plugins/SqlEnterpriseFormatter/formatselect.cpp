#include "formatselect.h"
#include "parser/ast/sqlitewith.h"

FormatSelect::FormatSelect(SqliteSelect* select) :
    select(select)
{
}

void FormatSelect::formatInternal()
{
    keywordToLineUp("SELECT");

    if (select->with)
        appendLinedUpPrefixedString("WITH", format(select->with));

    for (SqliteSelect::Core* core : select->coreSelects)
    {
        append(format(core));
        switch (core->compoundOp)
        {
            case SqliteSelect::CompoundOperator::UNION:
                appendKeyword(" UNION");
                newLine();
                break;
            case SqliteSelect::CompoundOperator::UNION_ALL:
                appendKeyword(" UNION ALL");
                newLine();
                break;
            case SqliteSelect::CompoundOperator::INTERSECT:
                appendKeyword(" INTERSECT");
                newLine();
                break;
            case SqliteSelect::CompoundOperator::EXCEPT:
                appendKeyword(" EXCEPT");
                newLine();
                break;
            case SqliteSelect::CompoundOperator::null:
                break;
        }
    }
}

void FormatSelect::resetInternal()
{
}


FormatSelectCore::FormatSelectCore(SqliteSelect::Core *core) :
    core(core)
{
}

void FormatSelectCore::formatInternal()
{
    keywordToLineUp("SELECT");

    if (core->valuesMode)
    {
        appendKeyword("VALUES ");
        return;
    }

    appendKeyword("SELECT ");
    if (core->distinctKw)
        appendKeyword("DISTINCT ");
    else if (core->allKw)
        appendKeyword("ALL ");

    append(formatDefList(core->resultColumns));

    if (core->from)
    {
        newLine();
        appendLinedUpKeyword("FROM");
        append(" ");
        append(format(core->from));
    }

    if (core->where)
    {
        newLine();
        appendLinedUpKeyword("WHERE");
        append(" ");
        append(format(core->where));
    }

    if (core->groupBy)
    {
        newLine();
        appendLinedUpKeyword("GROUP");
        append(" BY ");
        append(formatExprList(core->groupBy));
    }

    if (core->having)
    {
        newLine();
        appendLinedUpKeyword("HAVING");
        append(" ");
        append(format(core->having));
    }

    if (core->orderBy)
    {
        newLine();
        appendLinedUpKeyword("ORDER");
        append(" BY ");
        append(formatExprList(core->orderBy));
    }

    if (core->limit)
    {
        newLine();
        appendLinedUpKeyword("LIMIT");
        append(" ");
        append(format(core->limit));
    }
}

void FormatSelectCore::resetInternal()
{
}


FormatSelectCoreResultColumn::FormatSelectCoreResultColumn(SqliteSelect::Core::ResultColumn *resCol) :
    resCol(resCol)
{

}

void FormatSelectCoreResultColumn::formatInternal()
{
    if (resCol->star)
    {
        if (!resCol->table.isNull())
        {
            appendName(resCol->table);
            appendNameDot();
        }
        append("*");
    }
    else
    {
        append(format(resCol->expr));
        if (!resCol->alias.isNull())
        {
            if (resCol->asKw)
                appendKeyword(" AS");
        }
    }
}

void FormatSelectCoreResultColumn::resetInternal()
{
}
