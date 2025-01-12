#include "sqlitedelete.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqlitewith.h"

SqliteDelete::SqliteDelete()
{
    queryType = SqliteQueryType::Delete;
}

SqliteDelete::SqliteDelete(const SqliteDelete& other) :
    SqliteQuery(other), database(other.database), table(other.table), indexedByKw(other.indexedByKw), notIndexedKw(other.notIndexedKw),
    indexedBy(other.indexedBy)
{
    DEEP_COPY_FIELD(SqliteExpr, where);
    DEEP_COPY_FIELD(SqliteWith, with);
    DEEP_COPY_COLLECTION(SqliteResultColumn, returning);
}

SqliteDelete::SqliteDelete(const QString &name1, const QString &name2, const QString& alias, const QString& indexedByName, SqliteExpr *where, SqliteWith* with, const QList<SqliteResultColumn*>& returning, const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit)
    : SqliteDelete()
{
    init(name1, name2, alias, where, with, returning, orderBy, limit);
    this->indexedBy = indexedByName;
    this->indexedByKw = true;
}

SqliteDelete::SqliteDelete(const QString &name1, const QString &name2, const QString& alias, bool notIndexedKw, SqliteExpr *where, SqliteWith* with, const QList<SqliteResultColumn*>& returning, const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit)
    : SqliteDelete()
{
    init(name1, name2, alias, where, with, returning, orderBy, limit);
    this->notIndexedKw = notIndexedKw;
}

SqliteDelete::~SqliteDelete()
{
}

SqliteStatement* SqliteDelete::clone()
{
    return new SqliteDelete(*this);
}

QString SqliteDelete::getTable() const
{
    return table;
}

QString SqliteDelete::getDatabase() const
{
    return database;
}

QString SqliteDelete::getTableAlias() const
{
    return tableAlias;
}

QStringList SqliteDelete::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteDelete::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteDelete::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteDelete::getDatabaseTokensInStatement()
{
    if (tokensMap.contains("fullname"))
        return getDbTokenListFromFullname();

    if (tokensMap.contains("nm"))
        return extractPrintableTokens(tokensMap["nm"]);

    return TokenList();
}

QList<SqliteStatement::FullObject> SqliteDelete::getFullObjectsInStatement()
{
    QList<FullObject> result;
    if (!tokensMap.contains("fullname"))
        return result;

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

void SqliteDelete::init(const QString &name1, const QString &name2, const QString& alias, SqliteExpr *where, SqliteWith* with, const QList<SqliteResultColumn*>& returning,
                        const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit)
{
    this->where = where;
    if (where)
        where->setParent(this);

    this->with = with;
    if (with)
        with->setParent(this);

    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;

    this->tableAlias = alias;
    this->returning = returning;
    for (SqliteResultColumn*& retCol : this->returning)
        retCol->setParent(this);

    this->orderBy = orderBy;
    for (SqliteOrderBy*& order : this->orderBy)
        order->setParent(this);

    this->limit = limit;
    if (limit)
        limit->setParent(this);
}

TokenList SqliteDelete::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    if (with)
        builder.withStatement(with);

    builder.withKeyword("DELETE").withSpace().withKeyword("FROM").withSpace();
    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(table);
    if (!tableAlias.isNull())
        builder.withKeyword("AS").withSpace().withOther(tableAlias).withSpace();

    if (indexedByKw)
        builder.withSpace().withKeyword("INDEXED").withSpace().withKeyword("BY").withSpace().withOther(indexedBy);
    else if (notIndexedKw)
        builder.withSpace().withKeyword("NOT").withSpace().withKeyword("INDEXED");

    if (where)
        builder.withSpace().withKeyword("WHERE").withStatement(where);

    if (!returning.isEmpty())
    {
        builder.withKeyword("RETURNING");
        for (SqliteResultColumn*& retCol : returning)
            builder.withSpace().withStatement(retCol);
    }

    if (orderBy.size() > 0)
        builder.withSpace().withKeyword("ORDER").withSpace().withKeyword("BY").withStatementList(orderBy);

    if (limit)
        builder.withStatement(limit);

    builder.withOperator(";");

    return builder.build();
}
