#include "sqlitedroptable.h"
#include "sqlitequerytype.h"

SqliteDropTable::SqliteDropTable()
{
    queryType = SqliteQueryType::DropTable;
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
