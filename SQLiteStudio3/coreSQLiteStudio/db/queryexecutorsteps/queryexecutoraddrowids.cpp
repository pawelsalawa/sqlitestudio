#include "queryexecutoraddrowids.h"
#include "parser/ast/sqliteselect.h"
#include "selectresolver.h"
#include "common/utils_sql.h"
#include "parser/ast/sqlitecreatetable.h"
#include "schemaresolver.h"
#include <QDebug>

bool QueryExecutorAddRowIds::exec()
{
    if (context->noMetaColumns)
        return true;

    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    if (select->coreSelects.size() > 1)
        return true;

    if (select->coreSelects.first()->distinctKw || select->coreSelects.first()->valuesMode)
        return true;

    bool ok = true;
    addRowIdForTables(select.data(), ok);

    if (!ok)
    {
        qCritical() << "Error in QueryExecutorAddRowIds step.";
        return false;
    }

    // ...and putting it into parsed query, then update processed query
    select->rebuildTokens();
    updateQueries();

    return true;
}

QHash<SelectResolver::Table,QHash<QString,QString>> QueryExecutorAddRowIds::addRowIdForTables(SqliteSelect* select, bool& ok, bool isTopSelect)
{
    QHash<SelectResolver::Table,QHash<QString,QString>> rowIdColsMap;
    if (select->coreSelects.size() > 1)
        return rowIdColsMap;

    SqliteSelect::Core* core = select->coreSelects.first();

    if (core->groupBy.size() > 0)
        return rowIdColsMap;

    if (core->distinctKw)
        return rowIdColsMap;

    // Go trough subselects to add ROWID result columns there and collect rowId mapping to use here.
    foreach (SqliteSelect* subSelect, getSubSelects(core))
    {
        rowIdColsMap.unite(addRowIdForTables(subSelect, ok, false));
        if (!ok)
            return rowIdColsMap;
    }

    // Getting all tables we need to get ROWID for
    SelectResolver resolver(db, select->tokens.detokenize(), context->dbNameToAttach);
    resolver.resolveMultiCore = false; // multicore subselects result in not editable columns, skip them

    QSet<SelectResolver::Table> tables = resolver.resolveTables(core);
    foreach (const SelectResolver::Table& table, tables)
    {
        if (table.flags & (SelectResolver::FROM_COMPOUND_SELECT | SelectResolver::FROM_DISTINCT_SELECT | SelectResolver::FROM_GROUPED_SELECT))
            continue; // we don't get ROWID from compound, distinct or aggregated subselects

        if (!addResultColumns(core, table, rowIdColsMap, isTopSelect))
        {
            ok = false;
            return rowIdColsMap;
        }
    }
    return rowIdColsMap;
}

QList<SqliteSelect*> QueryExecutorAddRowIds::getSubSelects(SqliteSelect::Core* core)
{
    QList<SqliteSelect*> selects;
    if (!core->from)
        return selects;

    if (core->from->singleSource && core->from->singleSource->select)
        selects << core->from->singleSource->select;

    foreach (SqliteSelect::Core::JoinSourceOther* otherSource, core->from->otherSources)
    {
        if (!otherSource->singleSource->select)
            continue;

        selects << otherSource->singleSource->select;
    }

    return selects;
}

QHash<QString,QString> QueryExecutorAddRowIds::getNextColNames(const SelectResolver::Table& table)
{
    QHash<QString,QString> colNames;

    SchemaResolver resolver(db);
    SqliteQueryPtr query = resolver.getParsedObject(table.database, table.table, SchemaResolver::TABLE);
    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
    {
        qCritical() << "No CREATE TABLE object after parsing and casting in QueryExecutorAddRowIds::getNextColNames(). Cannot provide ROWID columns.";
        return colNames;
    }

    if (createTable->withOutRowId.isNull())
    {
        // It's a regular ROWID table
        colNames[getNextColName()] = "ROWID";
        return colNames;
    }

    SqliteStatement* primaryKey = createTable->getPrimaryKey();
    if (!primaryKey)
    {
        qCritical() << "WITHOUT ROWID table, but could not find    // Co PRIMARY KEY in QueryExecutorAddRowIds::getNextColNames().";
        return colNames;
    }

    SqliteCreateTable::Column::Constraint* columnConstr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(primaryKey);
    if (columnConstr)
    {
        colNames[getNextColName()] = dynamic_cast<SqliteCreateTable::Column*>(columnConstr->parentStatement())->name;
        return colNames;
    }

    SqliteCreateTable::Constraint* tableConstr = dynamic_cast<SqliteCreateTable::Constraint*>(primaryKey);
    if (tableConstr)
    {
        foreach (SqliteIndexedColumn* idxCol, tableConstr->indexedColumns)
            colNames[getNextColName()] = idxCol->name;

        return colNames;
    }

    qCritical() << "PRIMARY KEY that is neither table or column constraint. Should never happen (QueryExecutorAddRowIds::getNextColNames()).";
    return colNames;
}

bool QueryExecutorAddRowIds::addResultColumns(SqliteSelect::Core* core, const SelectResolver::Table& table,
                                        QHash<SelectResolver::Table,QHash<QString,QString>>& rowIdColsMap, bool isTopSelect)
{
    QHash<QString, QString> executorToRealColumns;
    if (rowIdColsMap.contains(table))
    {
        executorToRealColumns = rowIdColsMap[table]; // we already have resCol names from subselect
    }
    else
    {
        executorToRealColumns = getNextColNames(table);
        rowIdColsMap[table] = executorToRealColumns;
    }

    if (executorToRealColumns.size() == 0)
    {
        qCritical() << "No result column defined for a table in QueryExecutorAddRowIds::addResCols().";
        return false;
    }

    QHashIterator<QString,QString> it(executorToRealColumns);
    while (it.hasNext())
    {
        it.next();
        if (!addResultColumns(core, table, it.key(), it.value()))
            return false;
    }

    if (isTopSelect)
    {
        // Query executor result column description
        QueryExecutor::ResultRowIdColumnPtr queryExecutorResCol = QueryExecutor::ResultRowIdColumnPtr::create();
        queryExecutorResCol->dbName = table.originalDatabase;
        queryExecutorResCol->database = table.database;
        queryExecutorResCol->table = table.table;
        queryExecutorResCol->tableAlias = table.alias;
        queryExecutorResCol->queryExecutorAliasToColumn = executorToRealColumns;
        context->rowIdColumns << queryExecutorResCol;
    }

    return true;
}

bool QueryExecutorAddRowIds::addResultColumns(SqliteSelect::Core* core, const SelectResolver::Table& table, const QString& queryExecutorColumn,
                                        const QString& realColumn)
{
    SqliteSelect::Core::ResultColumn* resCol = new SqliteSelect::Core::ResultColumn();
    resCol->setParent(core);

    resCol->expr = new SqliteExpr();
    resCol->expr->setParent(resCol);

    resCol->expr->initId(realColumn);
    if (!table.alias.isNull())
    {
        resCol->expr->table = table.alias;
    }
    else
    {
        if (!table.database.isNull())
            resCol->expr->database = table.database;

        resCol->expr->table = table.table;
    }
    resCol->asKw = true;
    resCol->alias = queryExecutorColumn;

    core->resultColumns.insert(0, resCol);
    return true;
}
