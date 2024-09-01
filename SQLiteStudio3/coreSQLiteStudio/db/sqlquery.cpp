#include "sqlquery.h"
#include "db/sqlerrorcodes.h"
#include "common/utils_sql.h"
#include "common/unused.h"

SqlQuery::~SqlQuery()
{
}

bool SqlQuery::execute()
{
    if (queryArgs.userType() == QMetaType::QVariantHash)
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
        conditions << wrapObjIfNeeded(it.key()) + " = " + arg;
    }
}

const QHash<QString, QVariant>& RowIdConditionBuilder::getQueryArgs() const
{
    return queryArgs;
}

QString RowIdConditionBuilder::build()
{
    return conditions.join(" AND ");
}

/********************** SqlQueryError ************************/

class API_EXPORT SqlQueryError : public SqlQuery
{
    public:
        SqlQueryError(const QString& errorText, int errorCode);
        virtual ~SqlQueryError();

        QString getErrorText();
        int getErrorCode();
        QStringList getColumnNames();
        int columnCount();

    protected:
        SqlResultsRowPtr nextInternal();
        bool hasNextInternal();
        bool execInternal(const QList<QVariant>& args);
        bool execInternal(const QHash<QString, QVariant>& args);

    private:
        QString errorText;
        int errorCode = 0;
};

SqlQueryPtr SqlQuery::error(const QString& errorText, int errorCode)
{
    return SqlQueryPtr(new SqlQueryError(errorText, errorCode));
}

SqlQueryError::SqlQueryError(const QString& errorText, int errorCode) :
    errorText(errorText), errorCode(errorCode)
{
}

SqlQueryError::~SqlQueryError()
{
}

QString SqlQueryError::getErrorText()
{
    return errorText;
}

int SqlQueryError::getErrorCode()
{
    return errorCode;
}

QStringList SqlQueryError::getColumnNames()
{
    return QStringList();
}

int SqlQueryError::columnCount()
{
    return 0;
}

SqlResultsRowPtr SqlQueryError::nextInternal()
{
    return SqlResultsRowPtr();
}

bool SqlQueryError::hasNextInternal()
{
    return false;
}

bool SqlQueryError::execInternal(const QList<QVariant>& args)
{
    UNUSED(args);
    return false;
}

bool SqlQueryError::execInternal(const QHash<QString, QVariant>& args)
{
    UNUSED(args);
    return false;
}
