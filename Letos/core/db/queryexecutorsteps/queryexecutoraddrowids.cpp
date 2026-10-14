#include "queryexecutoraddrowids.h"
#include "parser/ast/sqliteselect.h"
#include "selectresolver.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "schemaresolver.h"
#include <QDebug>

static SelectResolver::Table normalizeRowIdTable(SelectResolver::Table table)
{
    if (table.database.compare("main", Qt::CaseInsensitive) == 0 &&
            (table.originalDatabase.isNull() || table.originalDatabase.compare("main", Qt::CaseInsensitive) == 0))
    {
        table.database = QString();
        table.originalDatabase = QString();
    }
    return table;
}

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
//    qDebug() << "before addrowid: " << context->processedQuery;
    select->rebuildTokens();
    updateQueries();
//    qDebug() << "after addrowid: " << context->processedQuery;

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
    for (SqliteSelect*& subSelect : getSubSelects(core))
    {
        auto subSelectRowIds = addRowIdForTables(subSelect, ok, false);
        if (!ok)
            return rowIdColsMap;

        // Expose inner row IDs under the source's alias at this SELECT boundary.
        // Otherwise separate references to the same table can overwrite each other.
        auto source = dynamic_cast<SqliteSelect::Core::SingleSource*>(subSelect->parentStatement());
        for (auto it = subSelectRowIds.constBegin(); it != subSelectRowIds.constEnd(); ++it)
        {
            SelectResolver::Table table = it.key();
            if (source && !source->alias.isNull())
            {
                table.pushTableAlias();
                table.tableAlias = source->alias;
            }
            rowIdColsMap[table] = it.value();
        }
    }
    select->rebuildTokens();

    // Getting all tables we need to get ROWID for
    SelectResolver resolver(db, select->tokens.detokenize(), context->dbNameToAttach);
    resolver.resolveMultiCore = false; // multicore subselects result in not editable columns, skip them

    QSet<SelectResolver::Table> tables = resolver.resolveTables(core);
    QHash<SelectResolver::Table,QHash<QString,QString>> currentRowIds;
    for (const SelectResolver::Table& table : tables)
    {
        if (table.flags & (SelectResolver::FROM_COMPOUND_SELECT | SelectResolver::FROM_DISTINCT_SELECT | SelectResolver::FROM_GROUPED_SELECT |
                           SelectResolver::FROM_CTE_SELECT | SelectResolver::FROM_TABLE_VALUED_FN | SelectResolver::FROM_RES_COL_SUBSELECT))
            continue; // we don't get ROWID from compound, distinct or aggregated subselects.

        // Tables from inside of view don't provide ROWID, if views were not expanded.
        if (!context->viewsExpanded && table.flags & SelectResolver::FROM_VIEW)
            continue;

        if (checkInWithClause(table, select->with))
            continue; // we don't get ROWID from WITH clause, as it's likely to be recurrent and difficult. TODO: support columns from WITH clause

        if (!addResultColumns(core, table, rowIdColsMap, isTopSelect))
        {
            ok = false;
            return rowIdColsMap;
        }
        SelectResolver::Table keyTable = normalizeRowIdTable(table);
        currentRowIds[keyTable] = rowIdColsMap.value(keyTable);
    }
    return currentRowIds;
}

QList<SqliteSelect*> QueryExecutorAddRowIds::getSubSelects(SqliteSelect::Core* core)
{
    QList<SqliteSelect*> selects;
    if (!core->from)
        return selects;

    if (core->from->singleSource && core->from->singleSource->select)
        selects << core->from->singleSource->select;

    for (SqliteSelect::Core::JoinSourceOther*& otherSource : core->from->otherSources)
    {
        if (!otherSource->singleSource->select)
            continue;

        selects << otherSource->singleSource->select;
    }

    return selects;
}

QHash<QString,QString> QueryExecutorAddRowIds::getNextColNames(const SelectResolver::Table& table)
{
    SchemaResolver resolver(db);
    QStringList rowIdCols = table.database.isNull() ?
                                resolver.getRowIdTableColumns(table.table) :
                                resolver.getRowIdTableColumns(table.database, table.table);

    QHash<QString,QString> colNames;
    for (const QString& colName : rowIdCols)
        colNames[getNextColName()] = colName;

    return colNames;
}

bool QueryExecutorAddRowIds::addResultColumns(SqliteSelect::Core* core, const SelectResolver::Table& table,
                                        QHash<SelectResolver::Table,QHash<QString,QString>>& rowIdColsMap, bool isTopSelect)
{
    SelectResolver::Table destilledTable = normalizeRowIdTable(table);

    SelectResolver::Table keyTable = destilledTable;

    // If selecting from named subselect, where table in that subselect has no alias, we need to match
    // Table by table&database, but excluding alias.
    if (!rowIdColsMap.contains(keyTable) && !keyTable.tableAlias.isEmpty())
    {
        keyTable.tableAlias = QString();
        if (!rowIdColsMap.contains(keyTable))
        {
            keyTable = destilledTable;
        }
    }

    // Aliased matching should be performed also against pushed (to old) aliases, due to multi-level subselects.
    if (!rowIdColsMap.contains(keyTable))
    {
        for (auto rowIdColsMapTable = rowIdColsMap.keyBegin(), end = rowIdColsMap.keyEnd(); rowIdColsMapTable != end; ++rowIdColsMapTable)
        {
            if (destilledTable.database.compare(rowIdColsMapTable->database, Qt::CaseInsensitive) != 0 ||
                    destilledTable.table.compare(rowIdColsMapTable->table, Qt::CaseInsensitive) != 0)
                continue;

            if (!table.oldTableAliases.contains(rowIdColsMapTable->tableAlias, Qt::CaseInsensitive))
                continue;

            keyTable = *rowIdColsMapTable;
        }
    }

    // Find ROWID column from inner SELECT, or create new column for the table.
    QHash<QString, QString> executorToRealColumns;
    bool aliasOnlyAsSelectColumn = false;
    if (rowIdColsMap.contains(keyTable))
    {
        executorToRealColumns = rowIdColsMap[keyTable]; // we already have resCol names from subselect
        aliasOnlyAsSelectColumn = true;
    }
    else
    {
        executorToRealColumns = getNextColNames(keyTable);
        rowIdColsMap[keyTable] = executorToRealColumns;
    }

    if (executorToRealColumns.size() == 0)
    {
        qCritical() << "No result column defined for a table in QueryExecutorAddRowIds::addResCols().";
        return false;
    }

    rowIdColsMap[destilledTable] = executorToRealColumns;

    QHashIterator<QString,QString> it(executorToRealColumns);
    while (it.hasNext())
    {
        it.next();
        if (!addResultColumns(core, destilledTable, it.key(), it.value(), aliasOnlyAsSelectColumn))
            return false;
    }

    if (isTopSelect)
    {
        // Query executor result column description
        QueryExecutor::ResultRowIdColumnPtr queryExecutorResCol = QueryExecutor::ResultRowIdColumnPtr::create();
        queryExecutorResCol->dbName = table.originalDatabase;
        queryExecutorResCol->database = table.database;
        queryExecutorResCol->table = table.table;
        queryExecutorResCol->tableAlias = table.tableAlias;
        queryExecutorResCol->queryExecutorAliasToColumn = executorToRealColumns;
        context->rowIdColumns << queryExecutorResCol;
    }

    return true;
}

bool QueryExecutorAddRowIds::checkInWithClause(const SelectResolver::Table &table, SqliteWith *with)
{
    if (!table.database.isNull() || !with)
        return false;

    SqliteWith::CommonTableExpression* cte = nullptr;
    QString nameToCompareWith = table.tableAlias.isNull() ? table.table : table.tableAlias;
    for (SqliteWith::CommonTableExpression*& cteItem : with->cteList)
    {
        if (cteItem->table == nameToCompareWith)
        {
            cte = cteItem;
            break;
        }
    }
    if (!cte)
        return false;

    return true;
}

bool QueryExecutorAddRowIds::addResultColumns(SqliteSelect::Core* core, const SelectResolver::Table& table, const QString& queryExecutorColumn,
                                        const QString& realColumn, bool aliasOnlyAsSelectColumn)
{
    SqliteSelect::Core::ResultColumn* resCol = new SqliteSelect::Core::ResultColumn();
    resCol->setParent(core);

    resCol->expr = new SqliteExpr();
    resCol->expr->setParent(resCol);

    if (aliasOnlyAsSelectColumn)
    {
        // We are re-querying this column from subselect, we already have it as an alias
        resCol->expr->initId(queryExecutorColumn);
    }
    else
    {
        resCol->expr->initId(realColumn);
        if (!table.tableAlias.isNull())
        {
            resCol->expr->table = table.tableAlias;
        }
        else
        {
            if (!table.database.isNull())
                resCol->expr->database = table.database;

            resCol->expr->table = table.table;
        }
    }
    resCol->asKw = true;
    resCol->alias = queryExecutorColumn;

    core->resultColumns << resCol;
    return true;
}
