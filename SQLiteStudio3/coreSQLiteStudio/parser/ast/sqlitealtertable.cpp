#include "sqlitealtertable.h"
#include "sqlitequerytype.h"
#include "common/global.h"

SqliteAlterTable::SqliteAlterTable()
{
    queryType = SqliteQueryType::AlterTable;
}

SqliteAlterTable::SqliteAlterTable(const SqliteAlterTable& other)
    : SqliteQuery(other), command(other.command), newName(other.newName), database(other.database), table(other.table),
    dropColumnName(other.dropColumnName), columnKw(other.columnKw)
{
    DEEP_COPY_FIELD(SqliteCreateTable::Column, newColumn);
}

SqliteAlterTable::SqliteAlterTable(const QString &name1, const QString &name2, const QString &newName)
    : SqliteAlterTable()
{
    command = Command::RENAME;
    initName(name1, name2);
    this->newName = newName;
}

SqliteAlterTable::SqliteAlterTable(const QString& name1, const QString& name2, bool columnKw, SqliteCreateTable::Column *column)
    : SqliteAlterTable()
{
    command = Command::ADD_COLUMN;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->newColumn = column;
    if (column)
        column->setParent(this);
}

SqliteAlterTable::SqliteAlterTable(const QString& name1, const QString& name2, bool columnKw, const QString& dropColumn)
    : SqliteAlterTable()
{
    command = Command::DROP_COLUMN;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->dropColumnName = dropColumn;
}

SqliteAlterTable::~SqliteAlterTable()
{
}

SqliteStatement* SqliteAlterTable::clone()
{
    return new SqliteAlterTable(*this);
}

QStringList SqliteAlterTable::getColumnsInStatement()
{
    QStringList list;
    if (!dropColumnName.isNull())
        list << dropColumnName;

    return list;
}

QStringList SqliteAlterTable::getTablesInStatement()
{
    QStringList list;
    if (!table.isNull())
        list << table;

    if (!newName.isNull())
        list << newName;

    return list;
}

QStringList SqliteAlterTable::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteAlterTable::getColumnTokensInStatement()
{
    if (command == Command::DROP_COLUMN && tokensMap.contains("nm"))
        return extractPrintableTokens(tokensMap["nm"]);

    return TokenList();
}

TokenList SqliteAlterTable::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteAlterTable::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteAlterTable::getFullObjectsInStatement()
{
    QList<FullObject> result;

    FullObject fullObj = getFullObjectFromFullname(FullObject::TABLE);
    if (fullObj.isValid())
        result << fullObj;

    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    return result;
}

void SqliteAlterTable::initName(const QString &name1, const QString &name2)
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;
}

TokenList SqliteAlterTable::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("ALTER").withSpace().withKeyword("TABLE").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(table).withSpace();

    switch (command) {
        case Command::RENAME:
        {
            builder.withKeyword("RENAME").withSpace().withKeyword("TO").withSpace().withOther(newName);
            break;
        }
        case Command::ADD_COLUMN:
        {
            builder.withKeyword("ADD").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withStatement(newColumn);
            break;
        }
        case Command::DROP_COLUMN:
        {
            builder.withKeyword("DROP").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withOther(dropColumnName);
            break;
        }
        case Command::null:
            break;
    }

    builder.withOperator(";");
    return builder.build();
}
