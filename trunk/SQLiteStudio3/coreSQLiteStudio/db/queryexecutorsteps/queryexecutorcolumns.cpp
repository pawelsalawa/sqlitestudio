#include "queryexecutorcolumns.h"
#include "utils_sql.h"
#include "parser/parser.h"
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
    QList<SelectResolver::Column> columns = resolver.resolve(select.data()).first();

    if (columns.size() == 0)
    {
        qWarning() << "SelectResolver could not resolve any column. Probably wrong table name entered by user, or something like that.";
        return false;
    }

    // Deleting old result columns and defining new ones
    SqliteSelect::Core* core = select->coreSelects.first();
    foreach (SqliteSelect::Core::ResultColumn* resCol, core->resultColumns)
        delete resCol;

    core->resultColumns.clear();

    // Defining result columns
    QueryExecutor::ResultColumnPtr resultColumn;
    SqliteSelect::Core::ResultColumn* resultColumnForSelect;
    foreach (const SelectResolver::Column& col, columns)
    {
        // Convert column to QueryExecutor result column
        resultColumn = getResultColumn(col);

        // Adding new result column to the query
        resultColumnForSelect = getResultColumnForSelect(resultColumn, col);
        resultColumnForSelect->setParent(core);
        core->resultColumns << resultColumnForSelect;

        if (!isRowIdColumnAlias(col.alias))
            context->resultColumns << resultColumn; // store it in context for later usage by any step
    }

    // Update query
    select->rebuildTokens();
    updateQueries();

//    qDebug() << context->processedQuery;

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
        resultColumn->queryExecutorAlias = getNextColName();
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

        resultColumn->database = resolvedColumn.originalDatabase;
        resultColumn->table = resolvedColumn.table;
        resultColumn->column = resolvedColumn.column;
        resultColumn->tableAlias = resolvedColumn.tableAlias;
        resultColumn->alias = resolvedColumn.alias;
        resultColumn->displayName = resolvedColumn.displayName;

        if (isRowIdColumnAlias(resultColumn->alias))
        {
            resultColumn->queryExecutorAlias = resultColumn->alias;
        }
        else
        {
            resultColumn->queryExecutorAlias = getNextColName();
        }
    }
    return resultColumn;
}

SqliteSelect::Core::ResultColumn* QueryExecutorColumns::getResultColumnForSelect(const QueryExecutor::ResultColumnPtr& resultColumn, const SelectResolver::Column& col)
{
    SqliteSelect::Core::ResultColumn* selectResultColumn = new SqliteSelect::Core::ResultColumn();

    QString colString = resultColumn->column;
    if (!resultColumn->expression)
        colString = wrapObjIfNeeded(colString, dialect);

    Parser parser(dialect);
    SqliteExpr* expr = parser.parseExpr(colString);
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

    selectResultColumn->asKw = true;
    selectResultColumn->alias = resultColumn->queryExecutorAlias;

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
    foreach (QueryExecutor::ResultRowIdColumnPtr rowIdColumn, context->rowIdColumns)
    {
        if (rowIdColumn->queryExecutorAliasToColumn.keys().contains(alias))
            return true;
    }
    return false;
}
