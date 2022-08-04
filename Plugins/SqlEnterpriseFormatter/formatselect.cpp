#include "formatselect.h"
#include "parser/ast/sqlitewith.h"
#include "parser/ast/sqlitewindowdefinition.h"

FormatSelect::FormatSelect(SqliteSelect* select) :
    select(select)
{
}

void FormatSelect::formatInternal()
{
    handleExplainQuery(select);
    markKeywordLineUp("SELECT");

    if (select->with)
        withStatement(select->with);

    for (SqliteSelect::Core* core : select->coreSelects)
    {
        switch (core->compoundOp)
        {
            case SqliteSelect::CompoundOperator::UNION:
                withNewLine().withKeyword("UNION").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::UNION_ALL:
            {
                if (core->valuesMode)
                    withListComma(FormatToken::Flag::NO_NEWLINE_BEFORE);
                else
                    withNewLine().withKeyword("UNION ALL").withNewLine();

                break;
            }
            case SqliteSelect::CompoundOperator::INTERSECT:
                withNewLine().withKeyword("INTERSECT").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::EXCEPT:
                withNewLine().withKeyword("EXCEPT").withNewLine();
                break;
            case SqliteSelect::CompoundOperator::null:
                break;
        }
        withStatement(core);
    }

    if (select->parentStatement() == nullptr) // it's not a subselect, it's top-level select
        withSemicolon();
}

FormatSelectCore::FormatSelectCore(SqliteSelect::Core *core) :
    core(core)
{
}

void FormatSelectCore::formatInternal()
{
    markKeywordLineUp("SELECT", "selectCore");

    if (core->valuesMode)
    {
        SqliteSelect* select = dynamic_cast<SqliteSelect*>(core->parentStatement());
        if (select->coreSelects.indexOf(core) == 0) // this is first core in series of cores of values mode of the SELECT
            withKeyword("VALUES");

        withParDefLeft().withStatementList(core->resultColumns).withParDefRight();
        return;
    }

    withKeyword("SELECT");
    if (core->distinctKw)
        withKeyword("DISTINCT");
    else if (core->allKw)
        withKeyword("ALL");

    withStatementList(core->resultColumns, "resultColumns");

    if (core->from)
        withNewLine().withLinedUpKeyword("FROM", "selectCore").withStatement(core->from, "source");

    if (core->where)
        withNewLine().withLinedUpKeyword("WHERE", "selectCore").withStatement(core->where, "conditions");

    if (core->groupBy.size() > 0)
        withNewLine().withLinedUpKeyword("GROUP", "selectCore").withKeyword("BY").withStatementList(core->groupBy, "grouping");

    if (core->having)
        withNewLine().withLinedUpKeyword("HAVING", "selectCore").withStatement(core->having, "having");

    if (core->windows.size() > 0)
    {
        withNewLine().withLinedUpKeyword("WINDOW", "selectCore");
        markKeywordLineUp("WINDOW", "selectWindow");
        withStatementList(core->windows, "selectWindow");
    }

    if (core->orderBy.size() > 0)
        withNewLine().withLinedUpKeyword("ORDER", "selectCore").withKeyword("BY").withStatementList(core->orderBy, "order");

    if (core->limit)
        withNewLine().withLinedUpKeyword("LIMIT", "selectCore").withStatement(core->limit, "limit");
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
            withIncrIndent("column");
            if (resCol->asKw)
                withKeyword("AS");

            withId(resCol->alias).withDecrIndent();
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
        }

        if (singleSource->indexedByKw)
            withKeyword("INDEXED").withKeyword("BY").withId(singleSource->indexedBy);
        else if (singleSource->notIndexedKw)
            withKeyword("NOT").withKeyword("INDEXED");
    }
    else if (!singleSource->funcName.isNull())
    {
        if (!singleSource->database.isNull())
            withId(singleSource->database).withIdDot();

        withId(singleSource->funcName).withParFuncLeft()
                .withStatementList(singleSource->funcParams, "funcArgs", FormatStatement::ListSeparator::EXPR_COMMA).withParFuncRight();

        if (!singleSource->alias.isNull())
        {
            if (singleSource->asKw)
                withKeyword("AS");

            withId(singleSource->alias);
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

    withNewLine();
    QStringList keywords;
    if (joinOp->naturalKw)
        keywords << "NATURAL";

    if (joinOp->leftKw || joinOp->fullKw || joinOp->rightKw)
    {
        if (joinOp->leftKw)
            keywords << "LEFT";
        else if (joinOp->fullKw)
            keywords << "FULL";
        else if (joinOp->rightKw)
            keywords << "RIGHT";

        if (joinOp->outerKw)
            keywords << "OUTER";
    }
    else if (joinOp->innerKw)
        keywords << "INNER";
    else if (joinOp->crossKw)
        keywords << "CROSS";

    keywords << "JOIN";

    if (keywords.size() == 0)
        return;

    for (const QString& kw : keywords)
        withKeyword(kw);

    if (cfg->SqlEnterpriseFormatter.NlAfterJoinStmt.get())
        withNewLine();
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
//    withStatement(joinSource->singleSource).withStatementList(joinSource->otherSources, "otherSources", ListSeparator::NONE);
    withStatement(joinSource->singleSource).withStatementList(joinSource->otherSources, QString(), ListSeparator::NONE);
}
