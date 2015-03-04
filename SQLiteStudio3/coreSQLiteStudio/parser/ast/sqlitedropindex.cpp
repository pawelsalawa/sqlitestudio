#include "sqlitedropindex.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteDropIndex::SqliteDropIndex()
{
    queryType = SqliteQueryType::DropIndex;
}

SqliteDropIndex::SqliteDropIndex(const SqliteDropIndex& other) :
    SqliteQuery(other), ifExistsKw(other.ifExistsKw), database(other.database), index(other.index)
{
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

SqliteStatement*SqliteDropIndex::clone()
{
    return new SqliteDropIndex(*this);
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


TokenList SqliteDropIndex::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("DROP").withSpace().withKeyword("INDEX").withSpace();

    if (ifExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withOther(index).withOperator(";");

    return builder.build();
}
