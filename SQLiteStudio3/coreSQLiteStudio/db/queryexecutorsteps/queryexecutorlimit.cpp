#include "queryexecutorlimit.h"
#include "parser/ast/sqlitelimit.h"
#include <QDebug>

bool QueryExecutorLimit::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    int page = queryExecutor->getPage();
    if (page < 0)
        return true; // no paging requested

    if (select->tokens.size() < 1)
        return true; // shouldn't happen, but if happens, quit gracefully

    quint64 limit = queryExecutor->getResultsPerPage();
    quint64 offset = limit * page;

    // The original query is last, so if it contained any %N strings,
    // they won't be replaced.
    QString newSelect = "SELECT * FROM (%3) LIMIT %1 OFFSET %2";
    newSelect = newSelect.arg(limit).arg(offset).arg(select->detokenize());

    int begin = select->tokens.first()->start;
    int length = select->tokens.last()->end - select->tokens.first()->start + 1;
    context->processedQuery = context->processedQuery.replace(begin, length, newSelect);
    return true;
}
