#include "queryexecutorwrapdistinctresults.h"

bool QueryExecutorWrapDistinctResults::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    SqliteSelect::Core* core = select->coreSelects.first();

    if (core->distinctKw || core->groupBy.size() > 0)
        wrapSelect(select.data());

    return true;
}

void QueryExecutorWrapDistinctResults::wrapSelect(SqliteSelect* select)
{
    // Trim tokens from right side
    TokenList origTokens = select->tokens;
    origTokens.trimRight();

    // Remove ; operator if present at the end
    while (origTokens.last()->type == Token::OPERATOR && origTokens.last()->value == ";")
        origTokens.removeLast();

    // Create new list
    TokenList tokens;
    tokens << TokenPtr::create(Token::KEYWORD, "SELECT");
    tokens << TokenPtr::create(Token::SPACE, " ");
    tokens << TokenPtr::create(Token::OPERATOR, "*");
    tokens << TokenPtr::create(Token::SPACE, " ");
    tokens << TokenPtr::create(Token::KEYWORD, "FROM");
    tokens << TokenPtr::create(Token::SPACE, " ");
    tokens << TokenPtr::create(Token::OPERATOR, "(");
    tokens += origTokens;
    tokens << TokenPtr::create(Token::OPERATOR, ")");
    tokens << TokenPtr::create(Token::OPERATOR, ";");

    select->tokens = tokens;
    updateQueries();
}
