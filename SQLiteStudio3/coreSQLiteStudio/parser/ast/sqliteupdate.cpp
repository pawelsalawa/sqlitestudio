#include "sqliteupdate.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqlitewith.h"
#include <QDebug>

SqliteUpdate::SqliteUpdate()
{
    queryType = SqliteQueryType::Update;
}

SqliteUpdate::SqliteUpdate(const SqliteUpdate& other) :
    SqliteQuery(other), onConflict(other.onConflict), database(other.database), table(other.table), indexedByKw(other.indexedByKw),
    notIndexedKw(other.notIndexedKw), indexedBy(other.indexedBy)
{
    // Special case of deep collection copy
    SqliteExpr* newExpr = nullptr;
    for (const ColumnAndValue& keyValue : other.keyValueMap)
    {
        newExpr = new SqliteExpr(*keyValue.second);
        newExpr->setParent(this);
        keyValueMap << ColumnAndValue(keyValue.first, newExpr);
    }

    DEEP_COPY_FIELD(SqliteExpr, where);
    DEEP_COPY_FIELD(SqliteWith, with);
    DEEP_COPY_FIELD(SqliteSelect::Core::JoinSource, from);
    DEEP_COPY_COLLECTION(SqliteResultColumn, returning);
}

SqliteUpdate::~SqliteUpdate()
{
}

SqliteUpdate::SqliteUpdate(SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QString& alias, bool notIndexedKw, const QString &indexedBy,
                           const QList<ColumnAndValue>& values, SqliteSelect::Core::JoinSource* from, SqliteExpr *where, SqliteWith* with,
                           const QList<SqliteResultColumn*>& returning, const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit)
    : SqliteUpdate()
{
    this->onConflict = onConflict;

    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;

    this->tableAlias = alias;
    this->indexedBy = indexedBy;
    this->indexedByKw = !(indexedBy.isNull());
    this->notIndexedKw = notIndexedKw;
    keyValueMap = values;

    this->from = from;
    if (from)
        from->setParent(this);

    this->where = where;
    if (where)
        where->setParent(this);

    this->with = with;
    if (with)
        with->setParent(this);

    for (ColumnAndValue& keyValue : keyValueMap)
        keyValue.second->setParent(this);

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

SqliteStatement* SqliteUpdate::clone()
{
    return new SqliteUpdate(*this);
}

SqliteExpr* SqliteUpdate::getValueForColumnSet(const QString& column)
{
    for (ColumnAndValue& keyValue : keyValueMap)
    {
        if (keyValue.first == column)
            return keyValue.second;
    }
    return nullptr;
}

QString SqliteUpdate::getTable() const
{
    return table;
}

QString SqliteUpdate::getDatabase() const
{
    return database;
}

QString SqliteUpdate::getTableAlias() const
{
    return tableAlias;
}

QStringList SqliteUpdate::getColumnsInStatement()
{
    QStringList columns;
    for (ColumnAndValue& keyValue : keyValueMap)
    {
        if (keyValue.first.userType() == QMetaType::QStringList)
            columns += keyValue.first.toStringList();
        else
            columns += keyValue.first.toString();
    }

    return columns;
}

QStringList SqliteUpdate::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteUpdate::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteUpdate::getColumnTokensInStatement()
{
    // This case is not simple. We only have "setlist" in tokensMap
    // and it contains entire: col = expr, col = expr.
    // In order to extract 'col' token, we go through all 'expr',
    // for each 'expr' we get its first token, then locate it
    // in entire "setlist", get back 2 tokens to get what's before "=".
    TokenList list;
    TokenList setListTokens = getTokenListFromNamedKey("setlist", -1);
    int setListTokensSize = setListTokens.size();
    int end;
    int start = 0;
    SqliteExpr* expr = nullptr;
    for (ColumnAndValue& keyValue : keyValueMap)
    {
        expr = keyValue.second;
        end = setListTokens.indexOf(expr->tokens[0]);
        if (end < 0 || end >= setListTokensSize)
        {
            qCritical() << "Went out of bounds while looking for column tokens in SqliteUpdate::getColumnTokensInStatement().";
            continue;
        }

        // Before expression tokens there will be only column(s) token(s)
        // and commans, and equal operator. Let's take only ID tokens, which are columns.
        list += setListTokens.mid(start, end - start - 1).filter(Token::OTHER);

        start = end + expr->tokens.size();
    }
    return list;
}

TokenList SqliteUpdate::getTableTokensInStatement()
{
    if (tokensMap.contains("fullname"))
        return getObjectTokenListFromFullname();

    return TokenList();
}

TokenList SqliteUpdate::getDatabaseTokensInStatement()
{
    if (tokensMap.contains("fullname"))
        return getDbTokenListFromFullname();

    if (tokensMap.contains("nm"))
        return extractPrintableTokens(tokensMap["nm"]);

    return TokenList();
}

QList<SqliteStatement::FullObject> SqliteUpdate::getFullObjectsInStatement()
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

TokenList SqliteUpdate::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    if (with)
        builder.withStatement(with);

    builder.withKeyword("UPDATE").withSpace();
    if (onConflict != SqliteConflictAlgo::null)
        builder.withKeyword("OR").withSpace().withKeyword(sqliteConflictAlgo(onConflict)).withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(table).withSpace();
    if (!tableAlias.isNull())
        builder.withKeyword("AS").withSpace().withOther(tableAlias).withSpace();

    if (indexedByKw)
        builder.withKeyword("INDEXED").withSpace().withKeyword("BY").withSpace().withOther(indexedBy).withSpace();
    else if (notIndexedKw)
        builder.withKeyword("NOT").withSpace().withKeyword("INDEXED").withSpace();

    builder.withKeyword("SET").withSpace();

    bool first = true;
    for (ColumnAndValue& keyVal : keyValueMap)
    {
        if (!first)
            builder.withOperator(",").withSpace();

        if (keyVal.first.userType() == QMetaType::QStringList)
            builder.withParLeft().withOtherList(keyVal.first.toStringList()).withParRight();
        else
            builder.withOther(keyVal.first.toString());

        builder.withSpace().withOperator("=").withStatement(keyVal.second);
        first = false;
    }

    if (from)
        builder.withSpace().withKeyword("FROM").withStatement(from);

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
