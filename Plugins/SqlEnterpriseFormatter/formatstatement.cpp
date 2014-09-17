#include "formatstatement.h"
#include "formatselect.h"
#include "formatexpr.h"
#include "formatlimit.h"
#include "formatraise.h"
#include "formatwith.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqliteexpr.h"
#include "parser/ast/sqlitelimit.h"
#include "parser/ast/sqliteraise.h"
#include "parser/ast/sqlitewith.h"
#include "sqlenterpriseformatter.h"
#include "common/utils_sql.h"
#include <QRegularExpression>
#include <QDebug>

#define FORMATTER_FACTORY_ENTRY(query, Type, FormatType) \
    if (dynamic_cast<Type*>(query)) \
        return new FormatType(dynamic_cast<Type*>(query))

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
    buildTokens();
    return detokenize();
}

void FormatStatement::setSelectedWrapper(NameWrapper wrapper)
{
    this->wrapper = wrapper;
}

void FormatStatement::buildTokens()
{
    cleanup();
    resetInternal();
    formatInternal();
}

FormatStatement *FormatStatement::forQuery(SqliteStatement *query)
{
    FormatStatement* stmt = nullptr;
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect, FormatSelect);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core, FormatSelectCore);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core::ResultColumn, FormatSelectCoreResultColumn);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core::JoinConstraint, FormatSelectCoreJoinConstraint);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core::JoinOp, FormatSelectCoreJoinOp);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core::JoinSource, FormatSelectCoreJoinSource);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core::JoinSourceOther, FormatSelectCoreJoinSourceOther);
    FORMATTER_FACTORY_ENTRY(query, SqliteSelect::Core::SingleSource, FormatSelectCoreSingleSource);
    FORMATTER_FACTORY_ENTRY(query, SqliteExpr, FormatExpr);
    FORMATTER_FACTORY_ENTRY(query, SqliteWith, FormatWith);
    FORMATTER_FACTORY_ENTRY(query, SqliteWith::CommonTableExpression, FormatWithCommonTableExpression);
    FORMATTER_FACTORY_ENTRY(query, SqliteRaise, FormatRaise);
    FORMATTER_FACTORY_ENTRY(query, SqliteLimit, FormatLimit);

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

FormatStatement& FormatStatement::withStatement(SqliteStatement* stmt, const QString& indentName)
{
    FormatStatement* formatStmt = forQuery(stmt, dialect, wrapper);
    if (!formatStmt)
        return *this;

    formatStmt->buildTokens();
    formatStmt->deleteTokens = false;

    if (!indentName.isNull())
        markAndKeepIndent(indentName);

    tokens += formatStmt->tokens;

    if (!indentName.isNull())
        decrIndent();

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
    line = "";
    lines.clear();
    indents.clear();
    namedIndents.clear();
    indents.push(0);
    if (deleteTokens)
    {
        for (FormatToken* token : tokens)
            delete token;
    }

    tokens.clear();
}

QString FormatStatement::detokenize()
{
    bool uppercaseKeywords = CFG_ADV_FMT.SqlEnterpriseFormatter.UppercaseKeywords.get();

    for (FormatToken* token : tokens)
    {
        switch (token->type)
        {
            case FormatToken::KEYWORD:
                applyIndent();
                applySpace(token->type);
                line += uppercaseKeywords ? token->value.toString().toUpper() : token->value.toString().toLower();
                break;
            case FormatToken::LINED_UP_KEYWORD:
                break;
            case FormatToken::ID:
                break;
            case FormatToken::OPERATOR:
                break;
            case FormatToken::STAR:
                break;
            case FormatToken::FLOAT:
                break;
            case FormatToken::STRING:
                break;
            case FormatToken::INTEGER:
                break;
            case FormatToken::BLOB:
                break;
            case FormatToken::BIND_PARAM:
                break;
            case FormatToken::ID_DOT:
                break;
            case FormatToken::PAR_DEF_LEFT:
                break;
            case FormatToken::PAR_DEF_RIGHT:
                break;
            case FormatToken::PAR_EXPR_LEFT:
                break;
            case FormatToken::PAR_EXPR_RIGHT:
                break;
            case FormatToken::PAR_FUNC_LEFT:
                break;
            case FormatToken::PAR_FUNC_RIGHT:
                break;
            case FormatToken::SEMICOLON:
                break;
            case FormatToken::COMMA_LIST:
                break;
            case FormatToken::COMMA_OPER:
                break;
            case FormatToken::FUNC_ID:
                break;
            case FormatToken::DATA_TYPE:
                break;
            case FormatToken::NEW_LINE:
                break;
            case FormatToken::INDENT_MARKER:
                break;
            case FormatToken::INCR_INDENT:
                break;
            case FormatToken::DECR_INDENT:
                break;
        }
        lastToken = token;

    }
    return lines.join("\n");
}

void FormatStatement::applyIndent()
{
    int indentToAdd = indents.top() - line.length();
    if (indentToAdd <= 0)
        return;

    line += SPACE.repeated(indentToAdd);
}

void FormatStatement::applySpace(FormatToken::Type type)
{
    if (lastToken && isSpaceExpectingType(type) && isSpaceExpectingType(lastToken->type))
        line += " ";
}

bool FormatStatement::isSpaceExpectingType(FormatStatement::FormatToken::Type type)
{
    switch (type)
    {
        case FormatToken::KEYWORD:
        case FormatToken::LINED_UP_KEYWORD:
        case FormatToken::ID:
        case FormatToken::FLOAT:
        case FormatToken::STRING:
        case FormatToken::INTEGER:
        case FormatToken::BLOB:
        case FormatToken::BIND_PARAM:
        case FormatToken::FUNC_ID:
        case FormatToken::DATA_TYPE:
            return true;
        case FormatToken::OPERATOR:
        case FormatToken::STAR:
        case FormatToken::ID_DOT:
        case FormatToken::PAR_DEF_LEFT:
        case FormatToken::PAR_DEF_RIGHT:
        case FormatToken::PAR_EXPR_LEFT:
        case FormatToken::PAR_EXPR_RIGHT:
        case FormatToken::PAR_FUNC_LEFT:
        case FormatToken::PAR_FUNC_RIGHT:
        case FormatToken::SEMICOLON:
        case FormatToken::COMMA_LIST:
        case FormatToken::COMMA_OPER:
        case FormatToken::NEW_LINE:
        case FormatToken::INDENT_MARKER:
        case FormatToken::INCR_INDENT:
        case FormatToken::DECR_INDENT:
            break;
    }
    return false;
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
