#include "sqlquery.h"
#include "db/sqlerrorcodes.h"

SqlQuery::~SqlQuery()
{
}

bool SqlQuery::execute()
{
    if (queryArgs.type() == QVariant::Hash)
        return execInternal(queryArgs.toHash());
    else
        return execInternal(queryArgs.toList());
}

SqlResultsRowPtr SqlQuery::next()
{
    if (preloaded)
    {
        if (preloadedRowIdx >= preloadedData.size())
            return SqlResultsRowPtr();

        return preloadedData[preloadedRowIdx++];
    }
    return nextInternal();
}

bool SqlQuery::hasNext()
{
    if (preloaded)
        return (preloadedRowIdx < preloadedData.size());

    return hasNextInternal();
}

qint64 SqlQuery::rowsAffected()
{
    return affected;
}

QList<SqlResultsRowPtr> SqlQuery::getAll()
{
    if (!preloaded)
        preload();

    return preloadedData;
}

void SqlQuery::preload()
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

QVariant SqlQuery::getSingleCell()
{
    SqlResultsRowPtr row = next();
    if (row.isNull())
        return QVariant();

    return row->value(0);
}

bool SqlQuery::isError()
{
    return getErrorCode() != 0;
}

bool SqlQuery::isInterrupted()
{
    return SqlErrorCode::isInterrupted(getErrorCode());
}

RowId SqlQuery::getInsertRowId()
{
    return insertRowId;
}

qint64 SqlQuery::getRegularInsertRowId()
{
    return insertRowId["ROWID"].toLongLong();
}

QString SqlQuery::getQuery() const
{
    return query;
}

void SqlQuery::setFlags(Db::Flags flags)
{
    this->flags = flags;
}

void SqlQuery::clearArgs()
{
    queryArgs = QVariant();
}

void SqlQuery::setArgs(const QList<QVariant>& args)
{
    queryArgs = args;
}

void SqlQuery::setArgs(const QHash<QString, QVariant>& args)
{
    queryArgs = args;
}


void RowIdConditionBuilder::setRowId(const RowId& rowId)
{
    static const QString argTempalate = QStringLiteral(":rowIdArg%1");

    QString arg;
    QHashIterator<QString,QVariant> it(rowId);
    int i = 0;
    while (it.hasNext())
    {
        it.next();
        arg = argTempalate.arg(i++);
        queryArgs[arg] = it.value();
        conditions << it.key() + " = " + arg;
    }
}

const QHash<QString, QVariant>& RowIdConditionBuilder::getQueryArgs()
{
    return queryArgs;
}

QString RowIdConditionBuilder::build()
{
    return conditions.join(" AND ");
}
