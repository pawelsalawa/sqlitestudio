#include "sqlitedropindex.h"
#include "sqlitequerytype.h"

SqliteDropIndex::SqliteDropIndex()
{
    queryType = SqliteQueryType::DropIndex;
}

SqliteDropIndex::SqliteDropIndex(bool ifExists, const QString &name1, const QString &name2)
    : SqliteDropIndex()
{
    this->ifExistsKw = ifExists;
    if (!name2.isNull())
    {
        database = name1;
        index = name2;
    }
    else
        index = name1;
}

QStringList SqliteDropIndex::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteDropIndex::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteDropIndex::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Index object
    FullObject fullObj = getFullObjectFromFullname(FullObject::INDEX);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}
