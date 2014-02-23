#include "sqlresults.h"
#include "db/sqlerrorcodes.h"

SqlResults::~SqlResults()
{
}

QList<SqlResultsRowPtr> SqlResults::getAll()
{
    if (preloaded)
        return preloadedData;

    QList<SqlResultsRowPtr> allRows;
    SqlResultsRowPtr row;
    while (!(row = next()).isNull())
        allRows << row;

    return allRows;
}

void SqlResults::preload()
{
    if (preloaded)
        return;

    preloadedData = getAll();
    preloaded = true;
}

QVariant SqlResults::getSingleCell()
{
    SqlResultsRowPtr row = next();
    if (row.isNull())
        return QVariant();

    return row->value(0);
}

bool SqlResults::isError()
{
    return getErrorCode() != -1;
}

bool SqlResults::isInterrupted()
{
    return SqlErrorCode::isInterrupted(getErrorCode());
}

RowId SqlResults::getInsertRowId()
{
    return insertRowId;
}
