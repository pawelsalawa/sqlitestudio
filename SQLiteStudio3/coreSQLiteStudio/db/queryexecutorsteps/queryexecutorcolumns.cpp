#include "queryexecutorcolumns.h"
#include "common/utils_sql.h"
#include "parser/parser.h"
#include "parser/parsererror.h"
#include <QDebug>

// TODO need to test if attach name resolving works here

bool QueryExecutorColumns::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
    {
        context->editionForbiddenReasons << QueryExecutor::EditionForbiddenReason::NOT_A_SELECT;
        return true;
    }

    // Resolving result columns of the select
    SelectResolver resolver(db, queryExecutor->getOriginalQuery(), context->dbNameToAttach);
    resolver.resolveMultiCore = true;
    QList<SelectResolver::Column> columns = resolver.resolve(select->coreSelects.first());

    if (resolver.hasErrors())
    {
        qWarning() << "SelectResolver could not resolve the SELECT properly:" << resolver.getErrors().join("\n");
        return false;
    }

    if (columns.size() == 0)
    {
        qWarning() << "SelectResolver could not resolve any column. Probably wrong table name entered by user, or something like that.";
        return false;
    }

    // Deleting old result columns and defining new ones
    SqliteSelect::Core* core = select->coreSelects.first();
    for (SqliteSelect::Core::ResultColumn*& resCol : core->resultColumns)
        delete resCol;

    core->resultColumns.clear();

    // Count total rowId columns
    for (QueryExecutor::ResultRowIdColumnPtr& rowIdCol : context->rowIdColumns)
        rowIdColNames += rowIdCol->queryExecutorAliasToColumn.keys();

    // Defining result columns
    QueryExecutor::ResultColumnPtr resultColumn;
    SqliteSelect::Core::ResultColumn* resultColumnForSelect = nullptr;
    bool rowIdColumn = false;
    QSet<QString> usedAliases;
    for (SelectResolver::Column& col : columns)
    {
        // Convert column to QueryExecutor result column
        resultColumn = getResultColumn(col);

        // Adding new result column to the query
        rowIdColumn = isRowIdColumn(col.alias);
        if (rowIdColumn && col.alias.contains(":"))
            continue; // duplicate ROWID column provided by SelectResolver. See isRowIdColumn() for details.

        resultColumnForSelect = getResultColumnForSelect(resultColumn, col, usedAliases);
        if (!resultColumnForSelect)
            return false;

        resultColumnForSelect->setParent(core);
        core->resultColumns << resultColumnForSelect;

        if (!rowIdColumn)
            context->resultColumns << resultColumn; // store it in context for later usage by any step
    }

    //qDebug() << "before: " << context->processedQuery;
    // Update query
    select->rebuildTokens();
    // #5179 does not seem to be needed anymore, because query executor alias is applied always in the main column loop above.
    // Keeping the commented reference here for a while, but to be removed in future (due to end of 2025).
    //wrapWithAliasedColumns(select.data());
    updateQueries();

    //qDebug() << "after:  " << context->processedQuery;

    return true;
}

QueryExecutor::ResultColumnPtr QueryExecutorColumns::getResultColumn(const SelectResolver::Column &resolvedColumn)
{
    QueryExecutor::ResultColumnPtr resultColumn = QueryExecutor::ResultColumnPtr::create();
    if (resolvedColumn.type == SelectResolver::Column::OTHER)
    {
        resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::EXPRESSION;
        resultColumn->displayName = resolvedColumn.displayName;
        resultColumn->column = resolvedColumn.column;
        resultColumn->alias = resolvedColumn.alias;
        resultColumn->expression = true;
    }
    else
    {
        if (isSystemTable(resolvedColumn.table))
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::SYSTEM_TABLE;

        if (resolvedColumn.flags & SelectResolver::FROM_COMPOUND_SELECT)
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::COMPOUND_SELECT;

        if (resolvedColumn.flags & SelectResolver::FROM_GROUPED_SELECT)
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::GROUPED_RESULTS;

        if (resolvedColumn.flags & SelectResolver::FROM_DISTINCT_SELECT)
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::DISTINCT_RESULTS;

        if (resolvedColumn.flags & SelectResolver::FROM_CTE_SELECT)
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::COMM_TAB_EXPR;

        if (resolvedColumn.flags & SelectResolver::FROM_VIEW)
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::VIEW_NOT_EXPANDED;

        if (resolvedColumn.flags & SelectResolver::FROM_RES_COL_SUBSELECT)
            resultColumn->editionForbiddenReasons << QueryExecutor::ColumnEditionForbiddenReason::RES_INLINE_SUBSEL;

        resultColumn->database = resolvedColumn.originalDatabase;
        resultColumn->table = resolvedColumn.table;
        resultColumn->column = resolvedColumn.column;
        resultColumn->tableAlias = resolvedColumn.tableAlias;
        resultColumn->alias = resolvedColumn.alias;
        resultColumn->displayName = resolvedColumn.displayName;
    }

    if (isRowIdColumnAlias(resultColumn->alias))
        resultColumn->queryExecutorAlias = resultColumn->alias;
    else
        resultColumn->queryExecutorAlias = getNextColName();

    return resultColumn;
}

SqliteSelect::Core::ResultColumn* QueryExecutorColumns::getResultColumnForSelect(const QueryExecutor::ResultColumnPtr& resultColumn, const SelectResolver::Column& col, QSet<QString> &usedAliases)
{
    SqliteSelect::Core::ResultColumn* selectResultColumn = new SqliteSelect::Core::ResultColumn();

    QString colString = resultColumn->column;
    if (col.aliasDefinedInSubQuery) // #2819 (id from old tracker was 2931)
        colString = wrapObjIfNeeded(col.alias);

    if (!resultColumn->expression && !col.aliasDefinedInSubQuery) // if alias defined in subquery, it's already wrapped
        colString = wrapObjIfNeeded(colString);

    Parser parser;
    SqliteExpr* expr = parser.parseExpr(colString);
    if (!expr)
    {
        qWarning() << "Could not parse result column expr:" << colString;
        if (parser.getErrors().size() > 0)
            qWarning() << "The error was:" << parser.getErrors().first()->getFrom() << ":" << parser.getErrors().first()->getMessage();

        delete selectResultColumn;
        return nullptr;
    }

    expr->setParent(selectResultColumn);
    selectResultColumn->expr = expr;

    if (!(col.flags & SelectResolver::FROM_ANONYMOUS_SELECT)) // anonymous subselect will result in no prefixes for result column
    {
        if (!resultColumn->tableAlias.isNull())
        {
            selectResultColumn->expr->table = resultColumn->tableAlias;
        }
        else if (!resultColumn->table.isNull())
        {
            if (!resultColumn->database.isNull())
            {
                if (context->dbNameToAttach.containsLeft(resultColumn->database, Qt::CaseInsensitive))
                    selectResultColumn->expr->database = context->dbNameToAttach.valueByLeft(resultColumn->database, Qt::CaseInsensitive);
                else
                    selectResultColumn->expr->database = resultColumn->database;
            }

            selectResultColumn->expr->table = resultColumn->table;
        }
    }

    if (!col.alias.isNull())
        selectResultColumn->expr->column = col.alias;

    // #5179 duplicate of the same source table columns (but with different table alias) requires executor alias to be applied
    // always and immediately here to get proper results.
    selectResultColumn->asKw = true;
    selectResultColumn->alias = resultColumn->queryExecutorAlias;

    // If this alias was already used we need to use sequential alias
    static_qstring(aliasTpl, "%1:%2");
    int nextAliasCounter = 1;
    QString aliasBase = selectResultColumn->alias;
    while (usedAliases.contains(selectResultColumn->alias))
        selectResultColumn->alias = aliasTpl.arg(aliasBase, QString::number(nextAliasCounter++));

    usedAliases += selectResultColumn->alias;

    return selectResultColumn;
}

QString QueryExecutorColumns::resolveAttachedDatabases(const QString &dbName)
{
    if (context->dbNameToAttach.containsRight(dbName, Qt::CaseInsensitive))
        return context->dbNameToAttach.valueByRight(dbName, Qt::CaseInsensitive);

    return dbName;
}

bool QueryExecutorColumns::isRowIdColumnAlias(const QString& alias)
{
    for (QueryExecutor::ResultRowIdColumnPtr& rowIdColumn : context->rowIdColumns)
    {
        if (rowIdColumn->queryExecutorAliasToColumn.keys().contains(alias))
            return true;
    }
    return false;
}

bool QueryExecutorColumns::isRowIdColumn(const QString& columnAlias)
{
    // In case of "SELECT * FROM (SELECT * FROM test);" the SelectResolver will return ROWID columns twice for each table listed,
    // because ROWID columns are recurrently handled by QueryExecutorAddRowIds step. We need to identify such columns and make them unique
    // in the final query.
    // Currently all columns have QueryExecutor aliased names, so we can assume they have unified alias name in form ResCol_N.
    // If SelectResolver returns any column like ResCol_N:X, then it means that the column is result of the query like above.
    // Note, that this assumption is correct for RowId columns. There can be columns aliased by user and those aliases won't be unified.
    QString aliasOnly = columnAlias;
    if (aliasOnly.contains(":"))
        aliasOnly = aliasOnly.left(aliasOnly.indexOf(":"));

    return rowIdColNames.contains(aliasOnly);
}
