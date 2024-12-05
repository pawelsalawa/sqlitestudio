
#include "queryexecutorlimit.h"
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
    static_qstring(selectTpl, "SELECT * FROM (%1) LIMIT %2 OFFSET %3");
    QString newSelect = selectTpl.arg(select->detokenize(), QString::number(limit), QString::number(offset));

    int begin = select->tokens.first()->start;
    int length = select->tokens.last()->end - select->tokens.first()->start + 1;
    context->processedQuery = context->processedQuery.replace(begin, length, newSelect);
    return true;
}
