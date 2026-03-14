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

SqliteAlterTable::~SqliteAlterTable()
{
}

void SqliteAlterTable::initRenameTable(const QString &name1, const QString &name2, const QString &newName)
{
    command = Command::RENAME;
    initName(name1, name2);
    this->newName = newName;
}

void SqliteAlterTable::initAddColumn(const QString& name1, const QString& name2, bool columnKw, SqliteCreateTable::Column *column)
{
    command = Command::ADD_COLUMN;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->newColumn = column;
    if (column)
        column->setParent(this);
}

void SqliteAlterTable::initDropColumn(const QString& name1, const QString& name2, bool columnKw, const QString& dropColumn)
{
    command = Command::DROP_COLUMN;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->dropColumnName = dropColumn;
}

void SqliteAlterTable::initRenameColumn(const QString& name1, const QString& name2, bool columnKw, const QString& oldColumnName, const QString& newColumnName)
{
    command = Command::RENAME_COLUMN;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->columnName = oldColumnName;
    this->newColumnName = newColumnName;
}

void SqliteAlterTable::initColumnSetNotNull(const QString& name1, const QString& name2, bool columnKw, const QString& colName, SqliteConflictAlgo algo)
{
    command = Command::SET_NOT_NULL;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->columnName = colName;
    this->onConflict = algo;
}

void SqliteAlterTable::initColumnDropNotNull(const QString& name1, const QString& name2, bool columnKw, const QString& colName)
{
    command = Command::DROP_NOT_NULL;
    initName(name1, name2);
    this->columnKw = columnKw;
    this->columnName = colName;
}

void SqliteAlterTable::initAddCheck(const QString& name1, const QString& name2, const QString& constrName, SqliteExpr* checkExpr, SqliteConflictAlgo algo)
{
    command = Command::ADD_CHECK;
    initName(name1, name2);
    this->constraintName = constrName;
    this->expr = checkExpr;
    this->onConflict = algo;
}

void SqliteAlterTable::initAddCheck(const QString& name1, const QString& name2, SqliteExpr* checkExpr, SqliteConflictAlgo algo)
{
    command = Command::ADD_CHECK;
    initName(name1, name2);
    this->expr = checkExpr;
    this->onConflict = algo;
}

void SqliteAlterTable::initDropConstraint(const QString& name1, const QString& name2, const QString& constrName)
{
    command = Command::DROP_CONSTRAINT;
    initName(name1, name2);
    this->constraintName = constrName;
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
    switch (command)
    {
        case SqliteAlterTable::Command::DROP_COLUMN:
        case SqliteAlterTable::Command::RENAME_COLUMN:
        case SqliteAlterTable::Command::SET_NOT_NULL:
        case SqliteAlterTable::Command::DROP_NOT_NULL:
        {
            if (tokensMap.contains("nm"))
                return extractPrintableTokens(tokensMap["nm"]);

            break;
        }
        case SqliteAlterTable::Command::RENAME:
        case SqliteAlterTable::Command::ADD_COLUMN:
        case SqliteAlterTable::Command::ADD_CHECK:
        case SqliteAlterTable::Command::DROP_CONSTRAINT:
        case SqliteAlterTable::Command::null:
            break;
    }

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

TokenList SqliteAlterTable::rebuildTokensFromContents(bool replaceStatementTokens) const
{
    StatementTokenBuilder builder(replaceStatementTokens);
    builder.withTokens(SqliteQuery::rebuildTokensFromContents(replaceStatementTokens));
    builder.withKeyword("ALTER").withSpace().withKeyword("TABLE").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(table).withSpace();

    switch (command)
    {
        case SqliteAlterTable::Command::SET_NOT_NULL:
            builder.withKeyword("ALTER").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withOther(columnName).withSpace().withKeyword("SET")
                    .withSpace().withKeyword("NOT").withSpace().withKeyword("NULL")
                    .withConflict(onConflict);

            break;
        case SqliteAlterTable::Command::DROP_NOT_NULL:
            builder.withKeyword("ALTER").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withOther(columnName).withSpace().withKeyword("DROP")
                    .withSpace().withKeyword("NOT").withSpace().withKeyword("NULL");
            break;
        case SqliteAlterTable::Command::ADD_CHECK:
            builder.withKeyword("ADD").withSpace();
            if (!constraintName.isNull())
                builder.withKeyword("CONSTRAINT").withSpace().withOther(constraintName).withSpace();

            builder.withKeyword("CHECK").withSpace().withParLeft().withStatement(expr).withParRight()
                    .withConflict(onConflict);

            break;
        case SqliteAlterTable::Command::DROP_CONSTRAINT:
            builder.withKeyword("DROP").withSpace().withKeyword("CONSTRAINT").withSpace().withOther(constraintName);
            break;
        case Command::RENAME:
            builder.withKeyword("RENAME").withSpace().withKeyword("TO").withSpace().withOther(newName);
            break;
        case Command::ADD_COLUMN:
            builder.withKeyword("ADD").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withStatement(newColumn);
            break;
        case Command::DROP_COLUMN:
            builder.withKeyword("DROP").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withOther(dropColumnName);
            break;
        case Command::RENAME_COLUMN:
            builder.withKeyword("RENAME").withSpace();
            if (columnKw)
                builder.withKeyword("COLUMN").withSpace();

            builder.withOther(columnName).withSpace().withKeyword("TO").withSpace().withOther(newColumnName);
            break;
        case Command::null:
            break;
    }

    builder.withOperator(";");
    return builder.build();
}
