#include "sqliteinsert.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "sqliteselect.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqlitewith.h"

SqliteInsert::SqliteInsert()
{
    queryType = SqliteQueryType::Insert;
}

SqliteInsert::SqliteInsert(const SqliteInsert& other) :
    SqliteQuery(other), replaceKw(other.replaceKw), defaultValuesKw(other.defaultValuesKw), onConflict(other.onConflict), database(other.database),
    table(other.table), columnNames(other.columnNames)
{
    DEEP_COPY_COLLECTION(SqliteExpr, values);
    DEEP_COPY_FIELD(SqliteSelect, select);
    DEEP_COPY_FIELD(SqliteWith, with);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns,
                           const QList<SqliteExpr *> &row, SqliteWith* with) :
    SqliteInsert()
{
    initName(name1, name2);
    initMode(replace, onConflict);
    columnNames = columns;
    values = row;

    this->with = with;
    if (with)
        with->setParent(this);

    foreach (SqliteExpr* expr, row)
        expr->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns,
                           SqliteSelect *select, SqliteWith* with) :
    SqliteInsert()
{
    initName(name1, name2);
    initMode(replace, onConflict);

    this->with = with;
    if (with)
        with->setParent(this);

    columnNames = columns;
    this->select = select;
    if (select)
        select->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns,
                           SqliteWith* with) :
    SqliteInsert()
{
    initName(name1, name2);
    initMode(replace, onConflict);

    this->with = with;
    if (with)
        with->setParent(this);

    columnNames = columns;
    defaultValuesKw = true;
}

SqliteInsert::~SqliteInsert()
{
}

QStringList SqliteInsert::getColumnsInStatement()
{
    QStringList columns;
    columns += columnNames;
    return columns;
}

QStringList SqliteInsert::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteInsert::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteInsert::getColumnTokensInStatement()
{
    TokenList list;
    foreach (TokenPtr token, getTokenListFromNamedKey("inscollist_opt", -1))
    {
        if (token->type != Token::OTHER && token->type != Token::KEYWORD)
            continue;

        list << token;
    }
    return list;
}

TokenList SqliteInsert::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteInsert::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteInsert::getFullObjectsInStatement()
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

void SqliteInsert::initName(const QString& name1, const QString& name2)
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;
}

void SqliteInsert::initMode(bool replace, SqliteConflictAlgo onConflict)
{
    replaceKw = replace;
    this->onConflict = onConflict;
}

TokenList SqliteInsert::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (with)
        builder.withStatement(with);

    if (replaceKw)
    {
        builder.withKeyword("REPLACE").withSpace();
    }
    else
    {
        builder.withKeyword("INSERT").withSpace();
        if (onConflict != SqliteConflictAlgo::null)
            builder.withKeyword("OR").withSpace().withKeyword(sqliteConflictAlgo(onConflict)).withSpace();
    }

    builder.withKeyword("INTO").withSpace();

    if (!database.isNull())
        builder.withOther(database, dialect).withSpace();

    builder.withOther(table, dialect).withSpace();

    if (defaultValuesKw)
    {
        builder.withKeyword("DEFAULT").withSpace().withKeyword("VALUES");
    }
    else
    {
        if (columnNames.size() > 0)
            builder.withParLeft().withOtherList(columnNames, dialect).withParRight().withSpace();

        if (select)
        {
            builder.withStatement(select);
        }
        else if (dialect == Dialect::Sqlite2) // Sqlite2 uses classic single row values
        {
            builder.withKeyword("VALUES").withSpace().withParLeft().withStatementList(values).withParRight();
        }
    }

    builder.withOperator(";");

    return builder.build();
}
