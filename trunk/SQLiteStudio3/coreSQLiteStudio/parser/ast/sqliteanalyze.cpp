#include "sqliteanalyze.h"
#include "sqlitequerytype.h"

SqliteAnalyze::SqliteAnalyze()
{
    queryType = SqliteQueryType::Analyze;
}

SqliteAnalyze::SqliteAnalyze(const QString &name1, const QString &name2)
    : SqliteAnalyze()
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;
}

QStringList SqliteAnalyze::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteAnalyze::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteAnalyze::getTableTokensInStatement()
{
    return getObjectTokenListFromNmDbnm();
}

TokenList SqliteAnalyze::getDatabaseTokensInStatement()
{
    return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteAnalyze::getFullObjectsInStatement()
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
