#include "queryexecutorcolumntype.h"

bool QueryExecutorColumnType::exec()
{
    if (context->noMetaColumns)
        return true;

    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    addTypeColumns(select.data());

    select->rebuildTokens();
    updateQueries();

    return true;
}

void QueryExecutorColumnType::addTypeColumns(SqliteSelect* select)
{
    for (const QueryExecutor::ResultColumnPtr& resCol : context->resultColumns)
    {
        QString nextCol = getNextColName();
        QString targetCol = resCol->queryExecutorAlias;

        for (SqliteSelect::Core* core : select->coreSelects)
        {
            SqliteSelect::Core::ResultColumn* realResCol = createRealTypeOfResCol(targetCol, nextCol);
            core->resultColumns << realResCol;
            realResCol->setParent(core);
        }

        context->typeColumnToResultColumnAlias[nextCol] = targetCol;
    }
}

SqliteSelect::Core::ResultColumn* QueryExecutorColumnType::createRealTypeOfResCol(const QString& targetCol, const QString& alias)
{
    SqliteExpr* targetColExpr = new SqliteExpr();
    targetColExpr->initId(targetCol);

    SqliteExpr* expr = new SqliteExpr();
    expr->initFunction("typeof", false, {targetColExpr});

    return new SqliteSelect::Core::ResultColumn(expr, true, alias);
}
