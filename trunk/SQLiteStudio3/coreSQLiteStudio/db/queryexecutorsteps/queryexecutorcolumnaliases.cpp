#include "queryexecutorcolumnaliases.h"
#include "common/strhash.h"

bool QueryExecutorColumnAliases::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
    {
        context->editionForbiddenReasons << QueryExecutor::EditionForbiddenReason::NOT_A_SELECT;
        return true;
    }

    TokenList columnTokens = select->getContextColumnTokens();
    StrHash<TokenList> nameToColumnTokens;
    for (const TokenPtr& token : columnTokens)
        nameToColumnTokens[token->value] << token;

    for (const QueryExecutor::ResultColumnPtr& resCol : context->resultColumns)
    {
        if (resCol->alias.isNull())
            continue;

        if (!nameToColumnTokens.contains(resCol->alias))
            continue;

        for (const TokenPtr& token : nameToColumnTokens[resCol->alias])
            token->value = resCol->queryExecutorAlias;
    }

    updateQueries();

    return true;
}
