#include "sqlitecopy.h"
#include "sqlitequerytype.h"

SqliteCopy::SqliteCopy()
{
    queryType = SqliteQueryType::Copy;
}

SqliteCopy::SqliteCopy(SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QString &name3, const QString &delim)
    : SqliteCopy()
{
    this->onConflict = onConflict;

    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;

    file = name3;
    delimiter = delim;
}

QStringList SqliteCopy::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteCopy::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteCopy::getTableTokensInStatement()
{
    return getObjectTokenListFromNmDbnm();
}

TokenList SqliteCopy::getDatabaseTokensInStatement()
{
    return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteCopy::getFullObjectsInStatement()
{
    QList<FullObject> result;

    FullObject fullObj = getFullObjectFromNmDbnm(FullObject::TABLE);
    if (fullObj.isValid())
        result << fullObj;

    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}
