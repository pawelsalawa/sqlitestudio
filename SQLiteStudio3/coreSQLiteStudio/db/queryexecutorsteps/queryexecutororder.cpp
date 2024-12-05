#include "queryexecutororder.h"
#include "common/utils_sql.h"
#include "parser/parser.h"
#include <QDebug>

bool QueryExecutorOrder::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    QueryExecutor::SortList sortOrder = queryExecutor->getSortOrder();
    if (sortOrder.size() == 0)
        return true; // no sorting requested

    if (select->tokens.size() < 1)
        return true; // shouldn't happen, but if happens, leave gracefully

    TokenList tokens = getOrderTokens(sortOrder);
    if (tokens.size() == 0)
    {
        // happens in cases like #4819
        queryExecutor->setSortOrder(QueryExecutor::SortList());
        return true;
    }

    static_qstring(selectTpl, "SELECT * FROM (%1) ORDER BY %2");
    QString newSelect = selectTpl.arg(select->detokenize(), tokens.detokenize());

    Parser parser;
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

TokenList QueryExecutorOrder::getOrderTokens(const QueryExecutor::SortList& sortOrder)
{
    TokenList tokens;
    QueryExecutor::ResultColumnPtr resCol;
    bool next = false;
    for (const QueryExecutor::Sort& sort : sortOrder)
    {
        if (sort.column >= context->resultColumns.size())
        {
            qCritical() << "There is less result columns in query executor context than index of requested sort column";
            return TokenList();
        }

        if (next)
        {
            tokens << TokenPtr::create(Token::OPERATOR, ",");
            tokens << TokenPtr::create(Token::SPACE, " ");
        }
        else
            next = true;

        resCol = context->resultColumns[sort.column];

        tokens << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias);
        tokens << TokenPtr::create(Token::SPACE, " ");

        if (sort.order == QueryExecutor::Sort::DESC)
        {
            tokens << TokenPtr::create(Token::KEYWORD, "DESC");
            tokens << TokenPtr::create(Token::SPACE, " ");
        }
        else
        {
            tokens << TokenPtr::create(Token::KEYWORD, "ASC");
            tokens << TokenPtr::create(Token::SPACE, " ");
        }
    }

    return tokens;
}
