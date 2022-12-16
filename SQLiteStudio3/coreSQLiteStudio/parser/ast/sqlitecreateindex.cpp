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
    DEEP_COPY_COLLECTION(SqliteOrderBy, indexedColumns);
}

SqliteCreateIndex::SqliteCreateIndex(bool unique, bool ifNotExists, const QString& name1, const QString& name2, const QString& name3,
                                     const QList<SqliteOrderBy*>& columns, SqliteExpr* where)
    : SqliteCreateIndex()
{
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

    for (SqliteOrderBy* idxCol : columns)
        idxCol->setParent(this);

    this->where = where;
}

SqliteCreateIndex::~SqliteCreateIndex()
{
}

SqliteStatement*SqliteCreateIndex::clone()
{
    return new SqliteCreateIndex(*this);
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
    return getTokenListFromNamedKey("nm2");
}

TokenList SqliteCreateIndex::getDatabaseTokensInStatement()
{
    return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteCreateIndex::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj;
    TokenList tableTokens = getTokenListFromNamedKey("nm2");
    if (tableTokens.size() > 0)
        fullObj = getFullObject(FullObject::TABLE, TokenPtr(), tableTokens[0]);

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
    fullObj = getFullObjectFromNmDbnm(FullObject::INDEX, "nm", "dbnm");

    if (fullObj.isValid())
        result << fullObj;

    return result;
}

TokenList SqliteCreateIndex::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("CREATE").withSpace();
    if (uniqueKw)
        builder.withKeyword("UNIQUE").withSpace();

    builder.withKeyword("INDEX").withSpace();

    if (ifNotExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(index).withSpace().withKeyword("ON").withSpace().withOther(table).withSpace().withParLeft()
            .withStatementList(indexedColumns).withParRight();

    if (where)
        builder.withSpace().withKeyword("WHERE").withStatement(where);

    builder.withOperator(";");

    return builder.build();
}

QList<SqliteOrderBy*> SqliteCreateIndex::toOrderColumns(const QList<SqliteIndexedColumn*>& columns)
{
    QList<SqliteOrderBy*> result;
    SqliteOrderBy* orderBy = nullptr;
    SqliteExpr* expr = nullptr;
    for (SqliteIndexedColumn* idxCol : columns)
    {
        orderBy = new SqliteOrderBy();
        orderBy->setParent(this);
        orderBy->expr = new SqliteExpr();
        orderBy->expr->setParent(orderBy);

        if (!idxCol->collate.isNull())
        {
            expr = new SqliteExpr();
            expr->initId(idxCol->name);
            expr->setParent(orderBy->expr);

            orderBy->expr->initCollate(expr, idxCol->collate);
        }
        else
        {
            orderBy->expr->initId(idxCol->name);
        }

        result << orderBy;
        delete idxCol;
    }
    return result;
}

QString SqliteCreateIndex::getTargetDatabase() const
{
    return database;
}

void SqliteCreateIndex::setTargetDatabase(const QString& database)
{
    this->database = database;
}

QString SqliteCreateIndex::getObjectName() const
{
    return index;
}

void SqliteCreateIndex::setObjectName(const QString& name)
{
    index = name;
}
