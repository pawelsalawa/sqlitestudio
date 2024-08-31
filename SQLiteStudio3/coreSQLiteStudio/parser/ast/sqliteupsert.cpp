#include "sqliteupsert.h"
#include "common/global.h"
#include "parser/ast/sqliteorderby.h"
#include "parser/ast/sqliteexpr.h"
#include "parser/statementtokenbuilder.h"
#include <QDebug>

SqliteUpsert::SqliteUpsert()
{
    doNothing = true;
}

SqliteUpsert::SqliteUpsert(const QList<SqliteOrderBy*>& conflictColumns, SqliteExpr* conflictWhere)
{
    this->conflictColumns = conflictColumns;
    this->conflictWhere = conflictWhere;

    if (this->conflictWhere)
        this->conflictWhere->setParent(this);

    for (SqliteOrderBy* idxCol : conflictColumns)
        idxCol->setParent(this);

    doNothing = true;
}

SqliteUpsert::SqliteUpsert(const QList<SqliteOrderBy*>& conflictColumns, SqliteExpr* conflictWhere, const QList<ColumnAndValue>& values, SqliteExpr* setWhere)
{
    this->conflictColumns = conflictColumns;
    this->conflictWhere = conflictWhere;
    this->keyValueMap = values;
    this->setWhere = setWhere;

    if (this->conflictWhere)
        this->conflictWhere->setParent(this);

    if (this->setWhere)
        this->setWhere->setParent(this);

    for (SqliteOrderBy* idxCol : conflictColumns)
        idxCol->setParent(this);

    doNothing = false;
}

SqliteUpsert::SqliteUpsert(const SqliteUpsert& other)
    : SqliteStatement(other), doNothing(other.doNothing)
{
    DEEP_COPY_COLLECTION(SqliteOrderBy, conflictColumns);

    // Special case of deep collection copy
    SqliteExpr* newExpr = nullptr;
    for (const ColumnAndValue& keyValue : other.keyValueMap)
    {
        newExpr = new SqliteExpr(*keyValue.second);
        newExpr->setParent(this);
        keyValueMap << ColumnAndValue(keyValue.first, newExpr);
    }

    DEEP_COPY_FIELD(SqliteExpr, conflictWhere);
    DEEP_COPY_FIELD(SqliteExpr, setWhere);
}

SqliteStatement* SqliteUpsert::clone()
{
    return new SqliteUpsert(*this);
}

TokenList SqliteUpsert::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("ON").withSpace().withKeyword("CONFLICT");
    if (!conflictColumns.isEmpty())
    {
        builder.withSpace().withParLeft().withStatementList(conflictColumns).withParRight();
        if (conflictWhere)
            builder.withSpace().withKeyword("WHERE").withStatement(conflictWhere);
    }

    builder.withSpace().withKeyword("DO").withSpace();

    if (doNothing)
    {
        builder.withKeyword("NOTHING");
    }
    else
    {
        builder.withKeyword("UPDATE").withSpace().withKeyword("SET").withSpace();
        bool first = true;
        for (const ColumnAndValue& keyVal : keyValueMap)
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

        if (setWhere)
            builder.withSpace().withKeyword("WHERE").withStatement(setWhere);
    }

    return builder.build();
}

QStringList SqliteUpsert::getColumnsInStatement()
{
    QStringList columns;
    for (const ColumnAndValue& keyValue : keyValueMap)
    {
        if (keyValue.first.userType() == QMetaType::QStringList)
            columns += keyValue.first.toStringList();
        else
            columns += keyValue.first.toString();
    }

    return columns;
}

TokenList SqliteUpsert::getColumnTokensInStatement()
{
    // Alrorithm same as in UPDATE. See comments there for details.
    TokenList list;
    TokenList setListTokens = getTokenListFromNamedKey("setlist", -1);
    int setListTokensSize = setListTokens.size();
    int end;
    int start = 0;
    SqliteExpr* expr = nullptr;
    for (const ColumnAndValue& keyValue : keyValueMap)
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
