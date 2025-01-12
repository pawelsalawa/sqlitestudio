#include "sqliteinsert.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "sqliteselect.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqlitewith.h"
#include "sqliteupsert.h"

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
    DEEP_COPY_FIELD(SqliteUpsert, upsert);
    DEEP_COPY_COLLECTION(SqliteResultColumn, returning);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QString& alias, const QList<QString> &columns,
                           const QList<SqliteExpr *> &row, SqliteWith* with, const QList<SqliteResultColumn*>& returning) :
    SqliteInsert()
{
    init(name1, name2, alias, replace, onConflict, returning);
    columnNames = columns;
    values = row;

    this->with = with;
    if (with)
        with->setParent(this);

    for (SqliteExpr* expr : row)
        expr->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QString& alias, const QList<QString> &columns,
                           SqliteSelect *select, SqliteWith* with, SqliteUpsert* upsert, const QList<SqliteResultColumn*>& returning) :
    SqliteInsert()
{
    init(name1, name2, alias, replace, onConflict, returning);

    this->with = with;
    if (with)
        with->setParent(this);

    this->upsert = upsert;
    if (upsert)
        upsert->setParent(this);

    columnNames = columns;
    this->select = select;
    if (select)
        select->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QString& alias, const QList<QString> &columns,
                           SqliteWith* with, const QList<SqliteResultColumn*>& returning) :
    SqliteInsert()
{
    init(name1, name2, alias, replace, onConflict, returning);

    this->with = with;
    if (with)
        with->setParent(this);

    columnNames = columns;
    defaultValuesKw = true;
}

SqliteInsert::~SqliteInsert()
{
}

SqliteStatement* SqliteInsert::clone()
{
    return new SqliteInsert(*this);
}

QString SqliteInsert::getTable() const
{
    return table;
}

QString SqliteInsert::getDatabase() const
{
    return database;
}

QString SqliteInsert::getTableAlias() const
{
    return tableAlias;
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
    for (TokenPtr& token : getTokenListFromNamedKey("idlist_opt", -1))
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
    if (tokensMap.contains("fullname"))
        return getDbTokenListFromFullname();

    if (tokensMap.contains("nm"))
        return extractPrintableTokens(tokensMap["nm"]);

    return TokenList();
}

QList<SqliteStatement::FullObject> SqliteInsert::getFullObjectsInStatement()
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

void SqliteInsert::init(const QString& name1, const QString& name2, const QString& alias, bool replace, SqliteConflictAlgo onConflict, const QList<SqliteResultColumn*>& returning)
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;

    this->tableAlias = alias;
    replaceKw = replace;
    this->onConflict = onConflict;

    this->returning = returning;
    for (SqliteResultColumn*& retCol : this->returning)
        retCol->setParent(this);
}

TokenList SqliteInsert::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
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
        builder.withOther(database).withOperator(".");

    builder.withOther(table).withSpace();
    if (!tableAlias.isNull())
        builder.withKeyword("AS").withSpace().withOther(tableAlias).withSpace();

    if (defaultValuesKw)
    {
        builder.withKeyword("DEFAULT").withSpace().withKeyword("VALUES");
    }
    else
    {
        if (columnNames.size() > 0)
            builder.withParLeft().withOtherList(columnNames).withParRight().withSpace();

        if (select)
        {
            builder.withStatement(select);
            if (upsert)
                builder.withSpace().withStatement(upsert);
        }
    }

    if (!returning.isEmpty())
    {
        builder.withKeyword("RETURNING");
        for (SqliteResultColumn*& retCol : returning)
            builder.withSpace().withStatement(retCol);
    }

    builder.withOperator(";");

    return builder.build();
}
