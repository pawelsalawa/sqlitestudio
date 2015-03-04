#include "sqlitereindex.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteReindex::SqliteReindex()
{
    queryType = SqliteQueryType::Reindex;
}

SqliteReindex::SqliteReindex(const SqliteReindex& other) :
    SqliteQuery(other), database(other.database), table(other.table)
{
}

SqliteReindex::SqliteReindex(const QString& name1, const QString& name2)
    : SqliteReindex()
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;
}

SqliteStatement*SqliteReindex::clone()
{
    return new SqliteReindex(*this);
}

QStringList SqliteReindex::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteReindex::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteReindex::getTableTokensInStatement()
{
    return getObjectTokenListFromNmDbnm();
}

TokenList SqliteReindex::getDatabaseTokensInStatement()
{
    return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteReindex::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj = getFullObjectFromNmDbnm(FullObject::TABLE);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}

TokenList SqliteReindex::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("REINDEX");
    if (!database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withOther(table).withOperator(";");

    return builder.build();
}
