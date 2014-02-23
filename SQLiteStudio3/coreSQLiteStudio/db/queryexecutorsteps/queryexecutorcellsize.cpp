#include "queryexecutorcellsize.h"
#include <QDebug>

bool QueryExecutorCellSize::exec()
{
    if (queryExecutor->getDataLengthLimit() < 0)
        return true;

    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    foreach (SqliteSelect::Core* core, select->coreSelects)
    {
        if (!applyDataLimit(select.data(), core))
            return false;
    }

    updateQueries();
    return true;
}

bool QueryExecutorCellSize::applyDataLimit(SqliteSelect* select, SqliteSelect::Core* core)
{
    if (core->tokensMap["selcollist"].size() == 0)
    {
        qCritical() << "No 'selcollist' in Select::Core. Cannot apply cell size limits.";
        return false;
    }

    bool first = true;
    TokenList tokens;

    foreach (const QueryExecutor::ResultRowIdColumnPtr& col, context->rowIdColumns)
    {
        if (!first)
            tokens += getSeparatorTokens();

        tokens += getNoLimitTokens(col);
        first = false;
    }

    foreach (const QueryExecutor::ResultColumnPtr& col, context->resultColumns)
    {
        if (!first)
            tokens += getSeparatorTokens();

        tokens += getLimitTokens(col);
        first = false;
    }

    // Wrapping original select with new select with limited columns
    select->tokens = wrapSelect(select->tokens, tokens);

    return true;
}

TokenList QueryExecutorCellSize::getLimitTokens(const QueryExecutor::ResultColumnPtr& resCol)
{
    TokenList newTokens;
    newTokens << TokenPtr::create(Token::OTHER, "substr")
              << TokenPtr::create(Token::PAR_LEFT, "(")
              << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias)
              << TokenPtr::create(Token::OPERATOR, ",")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::INTEGER, "1")
              << TokenPtr::create(Token::OPERATOR, ",")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::INTEGER, QString::number(queryExecutor->getDataLengthLimit()))
              << TokenPtr::create(Token::PAR_RIGHT, ")")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "AS")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias);
    return newTokens;
}

TokenList QueryExecutorCellSize::getNoLimitTokens(const QueryExecutor::ResultRowIdColumnPtr& resCol)
{
    TokenList newTokens;
    bool first = true;
    foreach (const QString& col, resCol->queryExecutorAliasToColumn.keys())
    {
        if (!first)
            newTokens += getSeparatorTokens();

        newTokens << TokenPtr::create(Token::OTHER, col);
        first = false;
    }
    return newTokens;
}

TokenList QueryExecutorCellSize::getSeparatorTokens()
{
    TokenList newTokens;
    newTokens << TokenPtr::create(Token::OPERATOR, ",");
    newTokens << TokenPtr::create(Token::SPACE, " ");
    return newTokens;
}

TokenList QueryExecutorCellSize::wrapSelect(const TokenList& selectTokens, const TokenList& resultColumnsTokens)
{
    TokenList newTokens;
    newTokens << TokenPtr::create(Token::KEYWORD, "SELECT")
              << TokenPtr::create(Token::SPACE, " ");
    newTokens += resultColumnsTokens;
    newTokens << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "FROM")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::PAR_LEFT, "(");
    newTokens += selectTokens;
    newTokens << TokenPtr::create(Token::PAR_RIGHT, ")");
    return newTokens;
}
