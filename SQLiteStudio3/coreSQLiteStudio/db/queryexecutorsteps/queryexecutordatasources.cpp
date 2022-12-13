#include "queryexecutordatasources.h"
#include "parser/ast/sqliteselect.h"
#include "selectresolver.h"
#include <QDebug>

bool QueryExecutorDataSources::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    if (select->coreSelects.size() > 1) // compound selects might have different collection of tables
        return true;

    if (select->coreSelects.first()->valuesMode)
        return true;

    SelectResolver resolver(db, select->tokens.detokenize(), context->dbNameToAttach);
    resolver.resolveMultiCore = false; // multicore subselects result in not editable columns, skip them

    SqliteSelect::Core* core = select->coreSelects.first();
    QSet<SelectResolver::Table> tables = resolver.resolveTables(core);
    for (const SelectResolver::Table& resolvedTable : tables)
    {
        if (resolvedTable.flags & SelectResolver::FROM_CTE_SELECT)
            continue;

        QueryExecutor::SourceTablePtr table = QueryExecutor::SourceTablePtr::create();
        table->database = resolvedTable.database;
        table->table = resolvedTable.table;
        table->alias = resolvedTable.tableAlias;
        context->sourceTables << table;
    }

    return true;
}
