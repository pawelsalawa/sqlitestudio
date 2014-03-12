#include "sqlitecreateindex.h"
#include "sqlitequerytype.h"
#include "sqliteindexedcolumn.h"
#include "parser/statementtokenbuilder.h"
#include "parser/ast/sqliteexpr.h"
#include "common/global.h"

SqliteCreateIndex::SqliteCreateIndex()
{
    queryType = SqliteQueryType::CreateIndex;
}

SqliteCreateIndex::SqliteCreateIndex(const SqliteCreateIndex& other) :
    SqliteQuery(other), uniqueKw(other.uniqueKw), ifNotExistsKw(other.ifNotExistsKw), database(other.database), index(other.index),
    table(other.table)
{
    DEEP_COPY_COLLECTION(SqliteIndexedColumn, indexedColumns);
}

SqliteCreateIndex::SqliteCreateIndex(bool unique, bool ifNotExists, const QString &name1, const QString &name2, const QString &name3,
                                     const QList<SqliteIndexedColumn *> &columns, SqliteConflictAlgo onConflict)
    : SqliteCreateIndex()
{
    // Constructor for SQLite 2
    uniqueKw = unique;
    ifNotExistsKw = ifNotExists;

    index = name1;

    if (!name3.isNull())
    {
        database = name2;
        table = name3;
    }
    else
        table = name2;

    this->onConflict = onConflict;
    this->indexedColumns = columns;

    foreach (SqliteIndexedColumn* idxCol, columns)
        idxCol->setParent(this);
}

SqliteCreateIndex::SqliteCreateIndex(bool unique, bool ifNotExists, const QString& name1, const QString& name2, const QString& name3,
                                     const QList<SqliteIndexedColumn*>& columns, SqliteExpr* where)
    : SqliteCreateIndex()
{
    // Constructor for SQLite 3
    uniqueKw = unique;
    ifNotExistsKw = ifNotExists;

    if (!name2.isNull())
    {
        database = name1;
        index = name2;
    }
    else
        index = name1;

    table = name3;
    this->indexedColumns = columns;

    foreach (SqliteIndexedColumn* idxCol, columns)
        idxCol->setParent(this);

    this->where = where;
}

SqliteCreateIndex::~SqliteCreateIndex()
{
}

QString SqliteCreateIndex::getTargetTable() const
{
    return table;
}

QStringList SqliteCreateIndex::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteCreateIndex::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteCreateIndex::getTableTokensInStatement()
{
    if (dialect == Dialect::Sqlite2)
        return getObjectTokenListFromNmDbnm("nm2", "dbnm");
    else
        return getTokenListFromNamedKey("nm2");
}

TokenList SqliteCreateIndex::getDatabaseTokensInStatement()
{
    if (dialect == Dialect::Sqlite2)
        return getDbTokenListFromNmDbnm("nm2", "dbnm");
    else
        return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteCreateIndex::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj;
    if (dialect == Dialect::Sqlite2)
        fullObj = getFullObjectFromNmDbnm(FullObject::TABLE, "nm2", "dbnm");
    else
    {
        TokenList tableTokens = getTokenListFromNamedKey("nm2");
        if (tableTokens.size() > 0)
            fullObj = getFullObject(FullObject::TABLE, TokenPtr(), tableTokens[0]);
    }

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    // Index object
    if (dialect == Dialect::Sqlite2)
    {
        TokenList tableTokens = getTokenListFromNamedKey("nm");
        if (tableTokens.size() > 0)
            fullObj = getFullObject(FullObject::INDEX, TokenPtr(), tableTokens[0]);
    }
    else
        fullObj = getFullObjectFromNmDbnm(FullObject::INDEX, "nm", "dbnm");

    return result;
}

TokenList SqliteCreateIndex::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withKeyword("CREATE").withSpace();
    if (uniqueKw)
        builder.withKeyword("UNIQUE").withSpace();

    builder.withKeyword("INDEX").withSpace();

    if (ifNotExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS").withSpace();

    if (dialect == Dialect::Sqlite2)
    {
        builder.withOther(index, dialect).withSpace().withKeyword("ON").withSpace();

        if (!database.isNull())
            builder.withOther(database, dialect).withOperator(".");

        builder.withOther(table, dialect).withSpace();
        builder.withParLeft().withStatementList(indexedColumns).withParRight();


        if (onConflict != SqliteConflictAlgo::null)
            builder.withSpace().withKeyword(sqliteConflictAlgo(onConflict));
    }
    else
    {
        if (!database.isNull())
            builder.withOther(database, dialect).withOperator(".");

        builder.withOther(index, dialect).withSpace().withKeyword("ON").withSpace().withOther(table, dialect).withSpace().withParLeft()
                .withStatementList(indexedColumns).withParRight();
    }

    if (where)
        builder.withSpace().withKeyword("WHERE").withStatement(where);

    return builder.build();
}
