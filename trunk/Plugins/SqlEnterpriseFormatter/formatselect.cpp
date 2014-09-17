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
        withStatement(select->with);

    for (SqliteSelect::Core* core : select->coreSelects)
    {
        withStatement(core);
        switch (core->compoundOp)
        {
            case SqliteSelect::CompoundOperator::UNION:
                withNewLine().withKeyword("UNION").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::UNION_ALL:
                withNewLine().withKeyword("UNION ALL").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::INTERSECT:
                withNewLine().withKeyword("INTERSECT").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::EXCEPT:
                withNewLine().withKeyword("EXCEPT").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::null:
                break;
        }
    }
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
        withKeyword("VALUES");
        return;
    }

    withKeyword("SELECT");
    if (core->distinctKw)
        withKeyword("DISTINCT");
    else if (core->allKw)
        withKeyword("ALL");

    withStatementList(core->resultColumns, "resultColumns");

    if (core->from)
        withNewLine().withLinedUpKeyword("FROM").withStatement(core->from, "source");

    if (core->where)
        withNewLine().withLinedUpKeyword("WHERE").withStatement(core->where, "conditions");

    if (core->groupBy.size() > 0)
        withNewLine().withLinedUpKeyword("GROUP").withKeyword("BY").withStatementList(core->groupBy, "grouping");

    if (core->having)
        withNewLine().withLinedUpKeyword("HAVING").withStatement(core->having, "having");

    if (core->orderBy.size() > 0)
        withNewLine().withLinedUpKeyword("ORDER").withKeyword("BY").withStatementList(core->orderBy, "order");

    if (core->limit)
        withNewLine().withLinedUpKeyword("LIMIT").withStatement(core->limit, "limit");
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
            withId(resCol->table).withIdDot();
        }
        withStar();
    }
    else
    {
        withStatement(resCol->expr, "column");
        if (!resCol->alias.isNull())
        {
            incrIndent("column");
            if (resCol->asKw)
                withKeyword("AS");

            withId(resCol->alias).decrIndent();
        }
    }
}

FormatSelectCoreSingleSource::FormatSelectCoreSingleSource(SqliteSelect::Core::SingleSource* singleSource) :
    singleSource(singleSource)
{
}

void FormatSelectCoreSingleSource::formatInternal()
{
    if (!singleSource->table.isNull())
    {
        if (!singleSource->database.isNull())
            withId(singleSource->database).withIdDot();

        withId(singleSource->table);

        if (!singleSource->alias.isNull())
        {
            if (singleSource->asKw)
                withKeyword("AS");

            withId(singleSource->alias);

            if (singleSource->indexedByKw)
                withKeyword("INDEXED").withKeyword("BY").withId(singleSource->indexedBy);
            else if (singleSource->notIndexedKw)
                withKeyword("NOT").withKeyword("INDEXED");
        }
    }
    else if (singleSource->select)
    {
        withParDefLeft().withStatement(singleSource->select).withParDefRight();
        if (!singleSource->alias.isNull())
        {
            if (singleSource->asKw)
                withKeyword("AS");

            withId(singleSource->alias);
        }
    }
    else
    {
        withParDefLeft().withStatement(singleSource->joinSource).withParDefRight();
    }
}

FormatSelectCoreJoinOp::FormatSelectCoreJoinOp(SqliteSelect::Core::JoinOp* joinOp) :
    joinOp(joinOp)
{
}

void FormatSelectCoreJoinOp::formatInternal()
{
    if (joinOp->comma)
    {
        withListComma();
        return;
    }

    switch (dialect)
    {
        case Dialect::Sqlite3:
        {
            if (joinOp->naturalKw)
                withKeyword("NATURAL");

            if (joinOp->leftKw)
            {
                withKeyword("LEFT");
                if (joinOp->outerKw)
                    withKeyword("OUTER");
            }
            else if (joinOp->innerKw)
                withKeyword("INNER");
            else if (joinOp->crossKw)
                withKeyword("CROSS");

            withKeyword("JOIN");
            break;
        }
        case Dialect::Sqlite2:
        {
            if (joinOp->naturalKw)
                withKeyword("NATURAL");

            if (joinOp->leftKw)
                withKeyword("LEFT");
            else if (joinOp->rightKw)
                withKeyword("RIGHT");
            else if (joinOp->fullKw)
                withKeyword("FULL");

            if (joinOp->innerKw)
                withKeyword("INNER");
            else if (joinOp->crossKw)
                withKeyword("CROSS");
            else if (joinOp->outerKw)
                withKeyword("OUTER");

            withKeyword("JOIN");
            break;
        }
    }
}

FormatSelectCoreJoinConstraint::FormatSelectCoreJoinConstraint(SqliteSelect::Core::JoinConstraint* joinConstr) :
    joinConstr(joinConstr)
{
}

void FormatSelectCoreJoinConstraint::formatInternal()
{
    if (joinConstr->expr)
        withKeyword("ON").withStatement(joinConstr->expr, "joinConstr");
    else
        withKeyword("USING").withParDefLeft().withIdList(joinConstr->columnNames).withParDefRight();
}

FormatSelectCoreJoinSourceOther::FormatSelectCoreJoinSourceOther(SqliteSelect::Core::JoinSourceOther* joinSourceOther) :
    joinSourceOther(joinSourceOther)
{
}

void FormatSelectCoreJoinSourceOther::formatInternal()
{
    withStatement(joinSourceOther->joinOp).withStatement(joinSourceOther->singleSource).withStatement(joinSourceOther->joinConstraint);
}


FormatSelectCoreJoinSource::FormatSelectCoreJoinSource(SqliteSelect::Core::JoinSource* joinSource) :
    joinSource(joinSource)
{
}

void FormatSelectCoreJoinSource::formatInternal()
{
    withStatement(joinSource->singleSource).withStatementList(joinSource->otherSources, "otherSources", ListSeparator::NONE);
}
