#include "queryexecutorfilter.h"
#include <QDebug>

bool QueryExecutorFilter::exec()
{
//    qDebug() << "filters:" << queryExecutor->getFilters();
//    qDebug() << "q1:" << context->processedQuery;
    if (queryExecutor->getFilters().trimmed().isEmpty())
        return true;

    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    if (select->tokens.size() < 1)
        return true; // shouldn't happen, but if happens, quit gracefully

    static_qstring(selectTpl, "SELECT * FROM (%1) WHERE %2");
    QString newSelect = selectTpl.arg(select->detokenize(), queryExecutor->getFilters());

    int begin = select->tokens.first()->start;
    int length = select->tokens.last()->end - select->tokens.first()->start + 1;
    context->processedQuery = context->processedQuery.replace(begin, length, newSelect);
//    qDebug() << "q2:" << context->processedQuery;
    return true;
}
