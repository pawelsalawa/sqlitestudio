#include "queryexecutorcolumntype.h"
#include "parser/parser.h"
#include <QDebug>
#include <QStringList>

bool QueryExecutorColumnType::exec()
{
    if (context->noMetaColumns)
        return true;

    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    static_qstring(selectTpl, "SELECT *, %1 FROM (%2)");

    QStringList columns = addTypeColumns();
    QString newSelect = selectTpl.arg(columns.join(", "), select->detokenize());

    Parser parser;
    if (!parser.parse(newSelect) || parser.getQueries().size() == 0)
    {
        qWarning() << "Could not parse SELECT after applying typeof(). Tried to parse query:\n" << newSelect;
        return false;
    }

    context->parsedQueries.removeLast();
    context->parsedQueries << parser.getQueries().first();

    updateQueries();

    select->rebuildTokens();
    updateQueries();

    return true;
}

QStringList QueryExecutorColumnType::addTypeColumns()
{
    static_qstring(typeOfColTpl, "typeof(%1) AS %2");
    QStringList typeColumns;
    for (QueryExecutor::ResultColumnPtr& resCol : context->resultColumns)
    {
        QString nextCol = getNextColName();
        QString targetCol = resCol->queryExecutorAlias;
        typeColumns << typeOfColTpl.arg(targetCol, nextCol);
        context->typeColumnToResultColumnAlias[nextCol] = targetCol;
    }
    return typeColumns;
}
