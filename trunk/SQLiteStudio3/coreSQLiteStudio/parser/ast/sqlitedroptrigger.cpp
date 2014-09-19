#include "sqlitedroptrigger.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteDropTrigger::SqliteDropTrigger()
{
    queryType = SqliteQueryType::DropTrigger;
}

SqliteDropTrigger::SqliteDropTrigger(bool ifExists, const QString &name1, const QString &name2)
    : SqliteDropTrigger()
{
    this->ifExistsKw = ifExists;

    if (name2.isNull())
        trigger = name1;
    else
    {
        database = name1;
        trigger = name2;
    }
}

QStringList SqliteDropTrigger::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteDropTrigger::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteDropTrigger::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj = getFullObjectFromFullname(FullObject::TRIGGER);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}


TokenList SqliteDropTrigger::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("DROP").withSpace().withKeyword("TRIGGER").withSpace();

    if (ifExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withOther(trigger).withOperator(";");

    return builder.build();
}
