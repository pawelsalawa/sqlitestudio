#include "formatstatement.h"
#include "formatselect.h"
#include "parser/ast/sqliteselect.h"
#include "sqlenterpriseformatter.h"
#include "common/utils_sql.h"
#include <QRegularExpression>
#include <QDebug>

#define FORMATTER_FACTORY_ENTRY(query, Type) \
    Sqlite##Type* local##Type##Ptr = dynamic_cast<Sqlite##Type*>(query); \
    if (local##Type##Ptr) \
        return new Format##Type(local##Type##Ptr)

const QString FormatStatement::SPACE = " ";

FormatStatement::FormatStatement()
{
    indents.push(0);
}

FormatStatement::~FormatStatement()
{
    cleanup();
}

QString FormatStatement::format()
{
    cleanup();
    resetInternal();
    formatInternal();
    return detokenize();
}

FormatStatement *FormatStatement::forQuery(SqliteStatement *query)
{
    FormatStatement* stmt = nullptr;
    FORMATTER_FACTORY_ENTRY(query, Select);

    if (stmt)
        stmt->dialect = query->dialect;
    else
        qWarning() << "Unhandled query passed to enterprise formatter!";

    return stmt;
}

void FormatStatement::resetInternal()
{
}

void FormatStatement::keywordToLineUp(const QString &keyword)
{
    int lgt = keyword.length();
    if (lgt >= kwLineUpPosition)
        kwLineUpPosition = lgt;
}

FormatStatement& FormatStatement::withKeyword(const QString& kw)
{
    withToken(FormatToken::KEYWORD, kw);
    return *this;
}

FormatStatement& FormatStatement::withLinedUpKeyword(const QString& kw)
{
    withToken(FormatToken::LINED_UP_KEYWORD, kw);
    return *this;
}

FormatStatement& FormatStatement::withId(const QString& id)
{
    withToken(FormatToken::ID, id);
    return *this;
}

FormatStatement& FormatStatement::withOperator(const QString& oper)
{
    withToken(FormatToken::OPERATOR, oper);
    return *this;
}

FormatStatement& FormatStatement::withIdDot()
{
    withToken(FormatToken::ID_DOT, ".");
    return *this;
}

FormatStatement& FormatStatement::withStar()
{
    withToken(FormatToken::STAR, "*");
    return *this;
}

FormatStatement& FormatStatement::withFloat(double value)
{
    withToken(FormatToken::FLOAT, value);
    return *this;
}

FormatStatement& FormatStatement::withInteger(qint64 value)
{
    withToken(FormatToken::INTEGER, value);
    return *this;
}

FormatStatement& FormatStatement::withString(const QString& value)
{
    withToken(FormatToken::STRING, value);
    return *this;
}

FormatStatement& FormatStatement::withBlob(const QString& value)
{
    withToken(FormatToken::BLOB, value);
    return *this;
}

FormatStatement& FormatStatement::withBindParam(const QString& name)
{
    withToken(FormatToken::BIND_PARAM, name);
    return *this;
}

FormatStatement& FormatStatement::withParDefLeft()
{
    withToken(FormatToken::PAR_DEF_LEFT, "(");
    return *this;
}

FormatStatement& FormatStatement::withParDefRight()
{
    withToken(FormatToken::PAR_DEF_RIGHT, ")");
    return *this;
}

FormatStatement& FormatStatement::withParExprLeft()
{
    withToken(FormatToken::PAR_EXPR_LEFT, "(");
    return *this;
}

FormatStatement& FormatStatement::withParExprRight()
{
    withToken(FormatToken::PAR_EXPR_RIGHT, ")");
    return *this;
}

FormatStatement& FormatStatement::withParFuncLeft()
{
    withToken(FormatToken::PAR_FUNC_LEFT, "(");
    return *this;
}

FormatStatement& FormatStatement::withParFuncRight()
{
    withToken(FormatToken::PAR_FUNC_RIGHT, ")");
    return *this;
}

FormatStatement& FormatStatement::withSemicolon()
{
    withToken(FormatToken::SEMICOLON, ";");
    return *this;
}

FormatStatement& FormatStatement::withListComma()
{
    withToken(FormatToken::COMMA_LIST, ",");
    return *this;
}

FormatStatement& FormatStatement::withCommaOper()
{
    withToken(FormatToken::COMMA_OPER, ",");
    return *this;
}

FormatStatement& FormatStatement::withFuncId(const QString& func)
{
    withToken(FormatToken::FUNC_ID, func);
    return *this;
}

FormatStatement& FormatStatement::withDataType(const QString& dataType)
{
    withToken(FormatToken::DATA_TYPE, dataType);
    return *this;
}

FormatStatement& FormatStatement::withNewLine()
{
    withToken(FormatToken::NEW_LINE, "\n");
    return *this;
}

FormatStatement& FormatStatement::withLiteral(const QVariant& value)
{
    if (value.isNull())
        return *this;

    bool ok;
    if (value.userType() == QVariant::Double)
    {
        value.toDouble(&ok);
        if (ok)
        {
            withFloat(value.toDouble());
            return *this;
        }
    }

    value.toInt(&ok);
    if (ok)
    {
        withInteger(value.toInt());
        return *this;
    }

    QString str = value.toString();
    if (str.startsWith("x'", Qt::CaseInsensitive) && str.endsWith("'"))
    {
        withBlob(str);
        return *this;
    }

    withString(str);
    return *this;
}

FormatStatement& FormatStatement::withStatement(const QString& contents, const QString& indentName)
{
    if (!indentName.isNull())
        markAndKeepIndent(indentName);

    withToken(contents.startsWith("\n") ? FormatToken::NEW_LINE_WITH_STATEMENT : FormatToken::STATEMENT, contents, false);

    if (!indentName.isNull())
        decrIndent();

    return *this;
}

FormatStatement& FormatStatement::withStatement(SqliteStatement* stmt, const QString& indentName)
{
    FormatStatement* formatStmt = forQuery(stmt, dialect, wrapper);
    if (!formatStmt)
        return *this;

    withStatement(formatStmt->format(), indentName);
    delete formatStmt;
    return *this;
}

FormatStatement& FormatStatement::withLinedUpStatement(int prefixLength, const QString& contents, const QString& indentName)
{
    if (!indentName.isNull())
        markAndKeepIndent(indentName);

    withToken(FormatToken::LINED_UP_STATEMENT, contents, prefixLength);

    if (!indentName.isNull())
        decrIndent();

    return *this;
}

FormatStatement& FormatStatement::withLinedUpStatement(int prefixLength, SqliteStatement* stmt, const QString& indentName)
{
    FormatStatement* formatStmt = forQuery(stmt, dialect, wrapper);
    if (!formatStmt)
        return *this;

    withLinedUpStatement(prefixLength, formatStmt->format(), indentName);
    delete formatStmt;
    return *this;
}

FormatStatement& FormatStatement::markIndentForNextToken(const QString& name)
{
    withToken(FormatToken::INDENT_MARKER, name);
    return *this;
}

FormatStatement& FormatStatement::markIndentForLastToken(const QString& name)
{
    if (tokens.size() == 0)
        return *this;

    tokens.last()->indentMarkName = name;
    return *this;
}

FormatStatement& FormatStatement::markAndKeepIndent(const QString& name)
{
    markIndentForNextToken(name);
    incrIndent(name);
    return *this;
}

FormatStatement& FormatStatement::incrIndent(const QString& name)
{
    withToken(FormatToken::INCR_INDENT, name);
    return *this;
}

FormatStatement& FormatStatement::decrIndent()
{
    withToken(FormatToken::DECR_INDENT, QString());
    return *this;
}

FormatStatement& FormatStatement::withIdList(const QStringList& names, const QString& indentName, ListSeparator sep)
{
    if (!indentName.isNull())
        markAndKeepIndent(indentName);

    bool first = true;
    foreach (const QString& name, names)
    {
        if (!first)
        {
            switch (sep)
            {
                case ListSeparator::COMMA:
                    withListComma();
                    break;
                case ListSeparator::NONE:
                    break;
            }
        }

        withId(name);
        first = false;
    }

    if (!indentName.isNull())
        decrIndent();

    return *this;
}

void FormatStatement::withToken(FormatStatement::FormatToken::Type type, const QVariant& value)
{
    withToken(type, value, 0);
}

void FormatStatement::withToken(FormatStatement::FormatToken::Type type, const QVariant& value, int lineUpPrefixLength)
{
    FormatToken* token = new FormatToken;
    token->type = type;
    token->value = value;
    token->lineUpPrefixLength = lineUpPrefixLength;
    tokens << token;
}

void FormatStatement::cleanup()
{
    kwLineUpPosition = 0;
    indents.clear();
    namedIndents.clear();
    indents.push(0);
    for (FormatToken* token : tokens)
        delete token;

    tokens.clear();
}

QString FormatStatement::detokenize()
{
    QString result = "";
    for (FormatToken* token : tokens)
    {
        result += token->value.toString();
    }
    return result;
}

FormatStatement* FormatStatement::forQuery(SqliteStatement* query, Dialect dialect, NameWrapper wrapper)
{
    FormatStatement* formatStmt = forQuery(query);
    if (formatStmt)
    {
        formatStmt->dialect = dialect;
        formatStmt->wrapper = wrapper;
    }
    return formatStmt;
}
