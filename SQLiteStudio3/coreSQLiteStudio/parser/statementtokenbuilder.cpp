#include "statementtokenbuilder.h"
#include "parser/ast/sqlitestatement.h"
#include "common/utils_sql.h"
#include "common/global.h"
#include <QVariant>

StatementTokenBuilder& StatementTokenBuilder::withKeyword(const QString& value)
{
    return with(Token::KEYWORD, value);
}

StatementTokenBuilder& StatementTokenBuilder::withOther(const QString& value)
{
    return withOther(value, true);
}

StatementTokenBuilder& StatementTokenBuilder::withOther(const QString& value, bool wrapIfNeeded)
{
    return with(Token::OTHER, wrapIfNeeded ? wrapObjIfNeeded(value) : value);
}

StatementTokenBuilder&StatementTokenBuilder::withStringPossiblyOther(const QString& value)
{
    if (value.contains("\""))
        return withOther(wrapObjIfNeeded(value));
    else
        return withOther(wrapObjName(value, NameWrapper::DOUBLE_QUOTE), false);
}

StatementTokenBuilder& StatementTokenBuilder::withOtherList(const QList<QString>& value, const QString& separator)
{
    bool first = true;
    for (const QString& str : value)
    {
        if (!first)
        {
            if (!separator.isEmpty())
                withOperator(separator);

            withSpace();
        }
        withOther(str);
        first = false;
    }
    return *this;
}

StatementTokenBuilder& StatementTokenBuilder::withOperator(const QString& value)
{
    return with(Token::OPERATOR, value);
}

StatementTokenBuilder& StatementTokenBuilder::withComment(const QString& value)
{
    return with(Token::COMMENT, value);
}

StatementTokenBuilder& StatementTokenBuilder::withFloat(const QVariant& value)
{
    return with(Token::FLOAT, doubleToString(value));
}

StatementTokenBuilder& StatementTokenBuilder::withInteger(qint64 value)
{
    return with(Token::INTEGER, QString::number(value));
}

StatementTokenBuilder& StatementTokenBuilder::withBindParam(const QString& value)
{
    return with(Token::BIND_PARAM, value);
}

StatementTokenBuilder& StatementTokenBuilder::withParLeft()
{
    return with(Token::PAR_LEFT, "(");
}

StatementTokenBuilder& StatementTokenBuilder::withParRight()
{
    return with(Token::PAR_RIGHT, ")");
}

StatementTokenBuilder& StatementTokenBuilder::withSpace()
{
    return with(Token::SPACE, " ");
}

StatementTokenBuilder& StatementTokenBuilder::withBlob(const QString& value)
{
    return with(Token::BLOB, value);
}

StatementTokenBuilder& StatementTokenBuilder::withString(const QString& value)
{
    return with(Token::STRING, wrapString(value));
}

StatementTokenBuilder& StatementTokenBuilder::withConflict(SqliteConflictAlgo onConflict)
{
    if (onConflict != SqliteConflictAlgo::null)
        return withSpace().withKeyword("ON").withSpace().withKeyword("CONFLICT")
                .withSpace().withKeyword(sqliteConflictAlgo(onConflict));

    return *this;
}

StatementTokenBuilder& StatementTokenBuilder::withSortOrder(SqliteSortOrder sortOrder)
{
    if (sortOrder != SqliteSortOrder::null)
        return withSpace().withKeyword(sqliteSortOrder(sortOrder));

    return *this;
}

StatementTokenBuilder& StatementTokenBuilder::withStatement(SqliteStatement* stmt)
{
    if (!stmt)
        return *this;

    stmt->rebuildTokens();
    if (stmt->tokens.size() > 0)
    {
        if (tokens.size() > 0 && !tokens.last()->isWhitespace() && tokens.last()->type != Token::PAR_LEFT)
            withSpace();

        tokens += stmt->tokens;
        tokens.trimRight(Token::OPERATOR, ";");
    }
    return *this;
}

StatementTokenBuilder& StatementTokenBuilder::withTokens(TokenList tokens)
{
    this->tokens += tokens;
    return *this;
}

StatementTokenBuilder& StatementTokenBuilder::withLiteralValue(const QVariant& value)
{
    if (value.isNull())
        return *this;

    if (value.userType() == QVariant::String)
    {
        withString(value.toString());
        return *this;
    }

    if (value.userType() == QVariant::ByteArray)
    {
        static_qstring(blobLiteral, "X'%1'");
        withBlob(blobLiteral.arg(QString::fromLatin1(value.toByteArray().toHex())));
        return *this;
    }

    bool ok;
    if (value.userType() == QVariant::Double)
    {
        value.toDouble(&ok);
        if (ok)
        {
            withFloat(value);
            return *this;
        }
    }

    qint64 longVal = value.toLongLong(&ok);
    if (ok)
    {
        withInteger(longVal);
        return *this;
    }

    withString(value.toString());
    return *this;
}

TokenList StatementTokenBuilder::build() const
{
    return tokens;
}

void StatementTokenBuilder::clear()
{
    tokens.clear();
    currentIdx = 0;
}

StatementTokenBuilder& StatementTokenBuilder::with(Token::Type type, const QString& value)
{
    int size = value.size();
    tokens << TokenPtr::create(type, value, currentIdx, currentIdx + size - 1);
    currentIdx += size;
    return *this;
}
