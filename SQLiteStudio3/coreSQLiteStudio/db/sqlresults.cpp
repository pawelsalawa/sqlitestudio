#include "sqlresults.h"
#include "db/sqlerrorcodes.h"

SqlResults::~SqlResults()
{
}

SqlResultsRowPtr SqlResults::next()
{
    if (preloaded)
    {
        if (preloadedRowIdx >= preloadedData.size())
            return SqlResultsRowPtr();

        return preloadedData[preloadedRowIdx++];
    }
    return nextInternal();
}

bool SqlResults::hasNext()
{
    if (preloaded)
        return (preloadedRowIdx < preloadedData.size());

    return hasNextInternal();
}

QList<SqlResultsRowPtr> SqlResults::getAll()
{
    if (!preloaded)
        preload();

    return preloadedData;
}

void SqlResults::preload()
{
    if (preloaded)
        return;

    QList<SqlResultsRowPtr> allRows;
    while (hasNextInternal())
        allRows << nextInternal();

    preloadedData = allRows;
    preloaded = true;
    preloadedRowIdx = 0;
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
    return getErrorCode() != 0;
}

bool SqlResults::isInterrupted()
{
    return SqlErrorCode::isInterrupted(getErrorCode());
}

RowId SqlResults::getInsertRowId()
{
    return insertRowId;
}
