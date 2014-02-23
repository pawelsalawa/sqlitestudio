#include "queryexecutorexplainmode.h"

bool QueryExecutorExplainMode::exec()
{
    if (!context->explainMode)
        return true; // explain mode disabled

    SqliteQueryPtr lastQuery = context->parsedQueries.last();

    if (!lastQuery)
        return true;

    // If last query wasn't in explain mode, switch it on
    if (!lastQuery->explain)
    {
        lastQuery->explain = true;
        lastQuery->tokens.prepend(TokenPtr::create(Token::SPACE, " "));
        lastQuery->tokens.prepend(TokenPtr::create(Token::KEYWORD, "EXPLAIN"));
    }

    // Limit queries to only last one
    context->parsedQueries.clear();
    context->parsedQueries << lastQuery;

    updateQueries();

    return true;
}
