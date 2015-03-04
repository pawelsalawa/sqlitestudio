#include "sqlitecreatevirtualtable.h"
#include "sqlitequerytype.h"

#include <parser/lexer.h>
#include <parser/statementtokenbuilder.h>

SqliteCreateVirtualTable::SqliteCreateVirtualTable()
{
    queryType = SqliteQueryType::CreateVirtualTable;
}

SqliteCreateVirtualTable::SqliteCreateVirtualTable(const SqliteCreateVirtualTable& other) :
    SqliteQuery(other), ifNotExistsKw(other.ifNotExistsKw), database(other.database), table(other.table), module(other.module), args(other.args)
{
}

SqliteCreateVirtualTable::SqliteCreateVirtualTable(bool ifNotExists, const QString &name1, const QString &name2, const QString &name3) :
    SqliteCreateVirtualTable()
{
    initName(name1, name2);
    this->ifNotExistsKw = ifNotExists;
    module = name3;
}

SqliteCreateVirtualTable::SqliteCreateVirtualTable(bool ifNotExists, const QString &name1, const QString &name2, const QString &name3, const QList<QString> &args) :
    SqliteCreateVirtualTable()
{
    initName(name1, name2);
    this->ifNotExistsKw = ifNotExists;
    module = name3;
    this->args = args;
}

SqliteStatement*SqliteCreateVirtualTable::clone()
{
    return new SqliteCreateVirtualTable(*this);
}

QStringList SqliteCreateVirtualTable::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteCreateVirtualTable::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteCreateVirtualTable::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteCreateVirtualTable::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteCreateVirtualTable::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj = getFullObjectFromFullname(FullObject::TABLE);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    return result;
}

void SqliteCreateVirtualTable::initName(const QString &name1, const QString &name2)
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;
}

TokenList SqliteCreateVirtualTable::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("CREATE").withSpace().withKeyword("VIRTUAL").withSpace().withKeyword("TABLE");
    if (ifNotExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withKeyword("USING").withSpace().withOther(module, dialect);
    if (!args.isEmpty())
    {
        builder.withSpace();
        int i = 0;
        for (const QString& arg : args)
        {
            if (i > 0)
                builder.withOperator(",").withSpace();

            builder.withTokens(Lexer::tokenize(arg, Dialect::Sqlite3));
            i++;
        }
    }

    builder.withOperator(";");

    return builder.build();
}
