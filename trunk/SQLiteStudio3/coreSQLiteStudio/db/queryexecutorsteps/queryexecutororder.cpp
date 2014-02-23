#include "queryexecutororder.h"
#include "utils_sql.h"
#include <QDebug>

bool QueryExecutorOrder::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    QueryExecutor::Sort sortOrder = queryExecutor->getSortOrder();
    if (sortOrder.column < 0)
        return true; // no sorting requested

    if (select->tokens.size() < 1)
        return true; // shouldn't happen, but if happens, quit gracefully

    TokenList tokens = getOrderTokens(sortOrder);
    if (tokens.size() == 0)
        return false;

    // Not using QString::arg() below, because it could replace %N strings in either SELECT or ORDER BY columns.
    QString newSelect = "SELECT * FROM (" + select->detokenize() + ") ORDER BY " + tokens.detokenize();

    int begin = select->tokens.first()->start;
    int length = select->tokens.last()->end - select->tokens.first()->start + 1;
    context->processedQuery = context->processedQuery.replace(begin, length, newSelect);
    return true;
}

TokenList QueryExecutorOrder::getOrderTokens(QueryExecutor::Sort sortOrder)
{
    TokenList tokens;

    if (sortOrder.column >= context->resultColumns.size())
    {
        qCritical() << "There is less result columns in query executor context than index of requested sort column";
        return tokens;
    }

    QString sortBy;
    QueryExecutor::ResultColumnPtr resCol = context->resultColumns[sortOrder.column];

    // We cannot simply sort by resCol.queryExecutorAlias, because it includes substr() and SQLite doesn't
    // sort it as user would normally entered by the user in the original query.
    if (!resCol->column.isNull())
    {
        if (!resCol->tableAlias.isNull())
            sortBy = wrapObjIfNeeded(resCol->tableAlias, dialect) + "." + wrapObjIfNeeded(resCol->column, dialect);
        else if (!resCol->database.isNull())
            sortBy = wrapObjIfNeeded(resCol->database, dialect) + "." + wrapObjIfNeeded(resCol->table, dialect) +
                     "." + wrapObjIfNeeded(resCol->column, dialect);
        else if (!resCol->table.isNull())
            sortBy = wrapObjIfNeeded(resCol->table, dialect) + "." + wrapObjIfNeeded(resCol->column, dialect);
        else
            sortBy = wrapObjIfNeeded(resCol->column, dialect);
    }
    else
        sortBy = resCol->displayName;

    if (sortBy.isEmpty())
    {
        qCritical() << "The column alias from query executor result columns is empty when tried to sort by it!";
        return tokens;
    }

    tokens << TokenPtr::create(Token::SPACE, " ");
    tokens << TokenPtr::create(Token::KEYWORD, "ORDER");
    tokens << TokenPtr::create(Token::SPACE, " ");
    tokens << TokenPtr::create(Token::KEYWORD, "BY");
    tokens << TokenPtr::create(Token::SPACE, " ");
    tokens << TokenPtr::create(Token::OTHER, sortBy);
    tokens << TokenPtr::create(Token::SPACE, " ");

    if (sortOrder.order == QueryExecutor::Sort::DESC)
    {
        tokens << TokenPtr::create(Token::KEYWORD, "DESC");
        tokens << TokenPtr::create(Token::SPACE, " ");
    }

    return tokens;
}
