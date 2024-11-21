#include "queryexecutorfilter.h"
#include <QDebug>

bool QueryExecutorFilter::exec()
{
//    qDebug() << "filters:" << queryExecutor->getFilters();
//    qDebug() << "q1:" << context->processedQuery;
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    if (select->tokens.size() < 1)
        return true; // shouldn't happen, but if happens, quit gracefully

    static_qstring(selectNoFiltersTpl, "SELECT * FROM (%1)");
    static_qstring(selectTpl, "SELECT * FROM (%1) WHERE %2");

    // Even without filters defined, the subselect is needed, because as it turns out (#5065)
    // certain column names are implicitly aliased when occur in subselects.
    // For example column "true" will be renamed to "columnN" if selected in subselect,
    // but will not be aliased if it's selected in 1-level select.
    // Because of that query executor provided inconsistent column aliases depending
    // on whether the filter was defined or not.
    QString newSelect = queryExecutor->getFilters().trimmed().isEmpty() ?
                selectNoFiltersTpl.arg(select->detokenize()) :
                selectTpl.arg(select->detokenize(), queryExecutor->getFilters());

    //qDebug() << "filter:" << queryExecutor->getFilters();

    int begin = select->tokens.first()->start;
    int length = select->tokens.last()->end - select->tokens.first()->start + 1;
    context->processedQuery = context->processedQuery.replace(begin, length, newSelect);
//    qDebug() << "q2:" << context->processedQuery;
    return true;
}
