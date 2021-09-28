#include "queryexecutorcellsize.h"
#include <QDebug>

bool QueryExecutorCellSize::exec()
{
    if (queryExecutor->getDataLengthLimit() < 0)
        return true;

    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    for (SqliteSelect::Core*& core : select->coreSelects)
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

    for (QueryExecutor::ResultColumnPtr& col : context->resultColumns)
    {
        if (!first)
            tokens += getSeparatorTokens();

        tokens += getLimitTokens(col);
        first = false;
    }

    for (QueryExecutor::ResultRowIdColumnPtr& col : context->rowIdColumns)
    {
        if (!first)
            tokens += getSeparatorTokens();

        tokens += getNoLimitTokens(col);
        first = false;
    }

    // Wrapping original select with new select with limited columns
    select->tokens = wrapSelect(select->tokens, tokens);

    return true;
}

TokenList QueryExecutorCellSize::getLimitTokens(const QueryExecutor::ResultColumnPtr& resCol)
{
    TokenList newTokens;

    // Not limiting cells that are not editable, because when later copying such cells, the full value is loaded
    // by re-executing entire query, which can be very slow (#4129).
    if (!resCol->editionForbiddenReasons.isEmpty())
    {
        newTokens << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias);
        return newTokens;
    }

    // CASE WHEN typeof(alias) IN ('real', 'integer', 'numeric', 'null') THEN alias ELSE substr(alias, 1, limit) END
    newTokens << TokenPtr::create(Token::KEYWORD, "CASE")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "WHEN")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::OTHER, "typeof")
              << TokenPtr::create(Token::PAR_LEFT, "(")
              << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias)
              << TokenPtr::create(Token::PAR_RIGHT, ")")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "IN")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::PAR_LEFT, "(")
              << TokenPtr::create(Token::STRING, "'real'")
              << TokenPtr::create(Token::OPERATOR, ",")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::STRING, "'integer'")
              << TokenPtr::create(Token::OPERATOR, ",")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::STRING, "'numeric'")
              << TokenPtr::create(Token::OPERATOR, ",")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::STRING, "'null'")
              << TokenPtr::create(Token::PAR_RIGHT, ")")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "THEN")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::OTHER, resCol->queryExecutorAlias)
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "ELSE")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::OTHER, "substr")
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
              << TokenPtr::create(Token::KEYWORD, "END")
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
    for (auto it = resCol->queryExecutorAliasToColumn.keyBegin(), end = resCol->queryExecutorAliasToColumn.keyEnd(); it != end; ++it)
    {
        if (!first)
            newTokens += getSeparatorTokens();

        newTokens << TokenPtr::create(Token::OTHER, *it);
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
