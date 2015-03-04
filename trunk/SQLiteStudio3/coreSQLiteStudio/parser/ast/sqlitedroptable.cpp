#include "sqlitedroptable.h"
#include "sqlitequerytype.h"
#include <parser/statementtokenbuilder.h>

SqliteDropTable::SqliteDropTable()
{
    queryType = SqliteQueryType::DropTable;
}

SqliteDropTable::SqliteDropTable(const SqliteDropTable& other) :
    SqliteQuery(other), ifExistsKw(other.ifExistsKw), database(other.database), table(other.table)
{
}

SqliteDropTable::SqliteDropTable(bool ifExists, const QString& name1, const QString& name2)
    : SqliteDropTable()
{
    this->ifExistsKw = ifExists;
    if (name2.isNull())
        this->table = name1;
    else
    {
        this->database = name1;
        this->table = name2;
    }
}

SqliteStatement* SqliteDropTable::clone()
{
    return new SqliteDropTable(*this);
}

QStringList SqliteDropTable::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteDropTable::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteDropTable::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteDropTable::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteDropTable::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj = getFullObjectFromFullname(FullObject::TABLE);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}

TokenList SqliteDropTable::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("DROP").withSpace().withKeyword("TABLE").withSpace();

    if (ifExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withOther(table).withOperator(";");

    return builder.build();
}
