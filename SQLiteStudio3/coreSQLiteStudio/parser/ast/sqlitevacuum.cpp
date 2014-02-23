#include "sqlitevacuum.h"
#include "sqlitequerytype.h"

SqliteVacuum::SqliteVacuum()
{
    queryType = SqliteQueryType::Vacuum;
}


SqliteVacuum::SqliteVacuum(const QString& name)
    : SqliteVacuum()
{
    if (!name.isNull())
        database = name;
}

QStringList SqliteVacuum::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteVacuum::getDatabaseTokensInStatement()
{
    return getTokenListFromNamedKey("nm");
}

QList<SqliteStatement::FullObject> SqliteVacuum::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Db object
    FullObject fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}
