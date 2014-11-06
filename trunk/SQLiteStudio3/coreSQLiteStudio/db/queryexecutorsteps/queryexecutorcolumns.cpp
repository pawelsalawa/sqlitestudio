#include "queryexecutorcolumns.h"
#include "common/utils_sql.h"
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
    for (SqliteSelect::Core::ResultColumn* resCol : core->resultColumns)
        delete resCol;

    core->resultColumns.clear();

    // Count total rowId columns
    int rowIdColCount = 0;
    for (const QueryExecutor::ResultRowIdColumnPtr& rowIdCol : context->rowIdColumns)
        rowIdColCount += rowIdCol->queryExecutorAliasToColumn.size();

    // Defining result columns
    QueryExecutor::ResultColumnPtr resultColumn;
    SqliteSelect::Core::ResultColumn* resultColumnForSelect;
    bool isRowIdColumn = false;
    int i = 0;
    for (const SelectResolver::Column& col : columns)
    {
        // Convert column to QueryExecutor result column
        resultColumn = getResultColumn(col);

        // Adding new result column to the query
        isRowIdColumn = (i < rowIdColCount);
        resultColumnForSelect = getResultColumnForSelect(resultColumn, col, isRowIdColumn);
        resultColumnForSelect->setParent(core);
        core->resultColumns << resultColumnForSelect;

        if (!isRowIdColumn)
            context->resultColumns << resultColumn; // store it in context for later usage by any step

        i++;
    }

    // Update query
    select->rebuildTokens();
    wrapWithAliasedColumns(select.data());
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

SqliteSelect::Core::ResultColumn* QueryExecutorColumns::getResultColumnForSelect(const QueryExecutor::ResultColumnPtr& resultColumn, const SelectResolver::Column& col, bool rowIdColumn)
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

    if (rowIdColumn || resultColumn->expression)
    {
        selectResultColumn->asKw = true;
        selectResultColumn->alias = resultColumn->queryExecutorAlias;
    }
    else if (!col.alias.isNull())
    {
        selectResultColumn->asKw = true;
        selectResultColumn->alias = col.alias;
    }

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

void QueryExecutorColumns::wrapWithAliasedColumns(SqliteSelect* select)
{
    // Wrap everything in a surrounding SELECT and given query executor alias to all columns this time
    TokenList sepTokens;
    sepTokens << TokenPtr::create(Token::OPERATOR, ",") << TokenPtr::create(Token::SPACE, " ");

    bool first = true;
    TokenList outerColumns;
    for (const QueryExecutor::ResultRowIdColumnPtr& rowIdColumn : context->rowIdColumns)
    {
        for (const QString& alias : rowIdColumn->queryExecutorAliasToColumn.keys())
        {
            if (!first)
                outerColumns += sepTokens;

            outerColumns << TokenPtr::create(Token::OTHER, alias);
            first = false;
        }
    }

    for (const QueryExecutor::ResultColumnPtr& resCol : context->resultColumns)
    {
        if (!first)
            outerColumns += sepTokens;

        if (resCol->expression)
        {
            // Just alias (below if-else), because expressions were provided with the query executor alias in the inner select
        }
        else if (!resCol->alias.isNull())
        {
            outerColumns << TokenPtr::create(Token::OTHER, wrapObjIfNeeded(resCol->alias, dialect));
            outerColumns << TokenPtr::create(Token::SPACE, " ");
            outerColumns << TokenPtr::create(Token::KEYWORD, "AS");
            outerColumns << TokenPtr::create(Token::SPACE, " ");
        }
        else if (!resCol->tableAlias.isNull())
        {
            outerColumns << TokenPtr::create(Token::OTHER, wrapObjIfNeeded(resCol->tableAlias, dialect));
            outerColumns << TokenPtr::create(Token::OPERATOR, ".");
            outerColumns << TokenPtr::create(Token::OTHER, wrapObjIfNeeded(resCol->column, dialect));
            outerColumns << TokenPtr::create(Token::SPACE, " ");
            outerColumns << TokenPtr::create(Token::KEYWORD, "AS");
            outerColumns << TokenPtr::create(Token::SPACE, " ");
        }
        else if (!resCol->column.isNull())
        {
            if (!resCol->table.isNull())
            {
                if (!resCol->database.isNull())
                {
                    outerColumns << TokenPtr::create(Token::OTHER, wrapObjIfNeeded(resCol->database, dialect));
                    outerColumns << TokenPtr::create(Token::OPERATOR, ".");
                }
                outerColumns << TokenPtr::create(Token::OTHER, wrapObjIfNeeded(resCol->table, dialect));
                outerColumns << TokenPtr::create(Token::OPERATOR, ".");
            }
            outerColumns << TokenPtr::create(Token::OTHER, wrapObjIfNeeded(resCol->column, dialect));
            outerColumns << TokenPtr::create(Token::SPACE, " ");
            outerColumns << TokenPtr::create(Token::KEYWORD, "AS");
            outerColumns << TokenPtr::create(Token::SPACE, " ");
        }
        else
        {
            qCritical() << "Not expression, but neither alias or column - in QueryExecutorColumns::wrapWithAliasedColumns. Fix it!";
        }
        outerColumns << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias);
        first = false;
    }

    select->tokens = wrapSelect(select->tokens, outerColumns);
}
