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
}

SqliteDelete::SqliteDelete(const QString &name1, const QString &name2, const QString& indexedByName, SqliteExpr *where, SqliteWith* with)
    : SqliteDelete()
{
    init(name1, name2, where, with);
    this->indexedBy = indexedByName;
    this->indexedByKw = true;
}

SqliteDelete::SqliteDelete(const QString &name1, const QString &name2, bool notIndexedKw, SqliteExpr *where, SqliteWith* with)
    : SqliteDelete()
{
    init(name1, name2, where, with);
    this->notIndexedKw = notIndexedKw;
}

SqliteDelete::~SqliteDelete()
{
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
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteDelete::getFullObjectsInStatement()
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

void SqliteDelete::init(const QString &name1, const QString &name2, SqliteExpr *where, SqliteWith* with)
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
}

TokenList SqliteDelete::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (with)
        builder.withStatement(with);

    builder.withKeyword("DELETE").withSpace().withKeyword("FROM").withSpace();
    if (!database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withOther(table, dialect);

    if (indexedByKw)
        builder.withSpace().withKeyword("INDEXED").withSpace().withKeyword("BY").withSpace().withOther(indexedBy, dialect);
    else if (notIndexedKw)
        builder.withSpace().withKeyword("NOT").withSpace().withKeyword("INDEXED");

    if (where)
        builder.withSpace().withKeyword("WHERE").withStatement(where);

    builder.withOperator(";");

    return builder.build();
}
