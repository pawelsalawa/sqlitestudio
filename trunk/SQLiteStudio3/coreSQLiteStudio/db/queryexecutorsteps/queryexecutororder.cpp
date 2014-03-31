#include "queryexecutororder.h"
#include "common/utils_sql.h"
#include "parser/parser.h"
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
        return true; // shouldn't happen, but if happens, leave gracefully

    TokenList tokens = getOrderTokens(sortOrder);
    if (tokens.size() == 0)
        return false;

    // Not using QString::arg() below, because it could replace %N strings in either SELECT or ORDER BY columns.
    QString newSelect = "SELECT * FROM (" + select->detokenize() + ") ORDER BY " + tokens.detokenize();

    Parser parser(dialect);
    if (!parser.parse(newSelect) || parser.getQueries().size() == 0)
    {
        qWarning() << "Could not parse SELECt after applying order. Tried to parse query:\n" << newSelect;
        return false;
    }

    context->parsedQueries.removeLast();
    context->parsedQueries << parser.getQueries().first();

    updateQueries();
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

    QueryExecutor::ResultColumnPtr resCol = context->resultColumns[sortOrder.column];

    tokens << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias);
    tokens << TokenPtr::create(Token::SPACE, " ");

    if (sortOrder.order == QueryExecutor::Sort::DESC)
    {
        tokens << TokenPtr::create(Token::KEYWORD, "DESC");
        tokens << TokenPtr::create(Token::SPACE, " ");
    }

    return tokens;
}
