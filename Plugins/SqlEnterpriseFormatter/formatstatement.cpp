#include "formatstatement.h"
#include "formatselect.h"
#include "formatexpr.h"
#include "formatlimit.h"
#include "formatraise.h"
#include "formatwith.h"
#include "formatcreatetable.h"
#include "formatcreatevirtualtable.h"
#include "formatforeignkey.h"
#include "formatcolumntype.h"
#include "formatindexedcolumn.h"
#include "formatinsert.h"
#include "formatempty.h"
#include "formataltertable.h"
#include "formatanalyze.h"
#include "formatattach.h"
#include "formatbegintrans.h"
#include "formatcommittrans.h"
#include "formatcreateindex.h"
#include "formatcreatetrigger.h"
#include "formatcreateview.h"
#include "formatdelete.h"
#include "formatupdate.h"
#include "formatdropindex.h"
#include "formatpragma.h"
#include "formatdroptable.h"
#include "formatdroptrigger.h"
#include "formatdropview.h"
#include "formatorderby.h"
#include "formatupsert.h"
#include "formatwindowdefinition.h"
#include "formatfilterover.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqliteexpr.h"
#include "parser/ast/sqlitelimit.h"
#include "parser/ast/sqliteraise.h"
#include "parser/ast/sqlitewith.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "parser/ast/sqliteforeignkey.h"
#include "parser/ast/sqlitecolumntype.h"
#include "parser/ast/sqliteindexedcolumn.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqliteemptyquery.h"
#include "parser/ast/sqlitealtertable.h"
#include "parser/ast/sqliteanalyze.h"
#include "parser/ast/sqliteattach.h"
#include "parser/ast/sqlitebegintrans.h"
#include "parser/ast/sqlitecommittrans.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqlitedropindex.h"
#include "parser/ast/sqlitedroptable.h"
#include "parser/ast/sqlitedroptrigger.h"
#include "parser/ast/sqlitedropview.h"
#include "parser/ast/sqliteorderby.h"
#include "parser/ast/sqlitepragma.h"
#include "parser/ast/sqliteupsert.h"
#include "parser/ast/sqlitewindowdefinition.h"
#include "parser/ast/sqlitefilterover.h"
#include "sqlenterpriseformatter.h"
#include "common/utils_sql.h"
#include "common/global.h"
#include <QRegularExpression>
#include <QDebug>
#include <QtGlobal>

#define FORMATTER_FACTORY_ENTRY(query, Type, FormatType) \
    if (dynamic_cast<Type*>(query)) \
        return new FormatType(dynamic_cast<Type*>(query))

const QString FormatStatement::SPACE = " ";
const QString FormatStatement::NEWLINE = "\n";
qint64 FormatStatement::nameSeq = 0;

FormatStatement::FormatStatement()
{
    static_qstring(nameTpl, "statement_%1");

    indents.push(0);
    statementName = nameTpl.arg(QString::number(nameSeq++));
}

FormatStatement::~FormatStatement()
{
    cleanup();
}

QString FormatStatement::format()
{
    buildTokens();
    return detokenize() + NEWLINE; // extra space when formatting multiple top level (not in CREATE TRIGGER) queries
}

void FormatStatement::setSelectedWrapper(NameWrapper wrapper)
{
    this->wrapper = wrapper;
}

void FormatStatement::setConfig(Cfg::SqlEnterpriseFormatterConfig* cfg)
{
    this->cfg = cfg;
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
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateTable, FormatCreateTable);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateTable::Column, FormatCreateTableColumn);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateTable::Column::Constraint, FormatCreateTableColumnConstraint);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateTable::Constraint, FormatCreateTableConstraint);
    FORMATTER_FACTORY_ENTRY(query, SqliteForeignKey, FormatForeignKey);
    FORMATTER_FACTORY_ENTRY(query, SqliteForeignKey::Condition, FormatForeignKeyCondition);
    FORMATTER_FACTORY_ENTRY(query, SqliteColumnType, FormatColumnType);
    FORMATTER_FACTORY_ENTRY(query, SqliteIndexedColumn, FormatIndexedColumn);
    FORMATTER_FACTORY_ENTRY(query, SqliteInsert, FormatInsert);
    FORMATTER_FACTORY_ENTRY(query, SqliteUpsert, FormatUpsert);
    FORMATTER_FACTORY_ENTRY(query, SqliteEmptyQuery, FormatEmpty);
    FORMATTER_FACTORY_ENTRY(query, SqliteAlterTable, FormatAlterTable);
    FORMATTER_FACTORY_ENTRY(query, SqliteAnalyze, FormatAnalyze);
    FORMATTER_FACTORY_ENTRY(query, SqliteAttach, FormatAttach);
    FORMATTER_FACTORY_ENTRY(query, SqliteBeginTrans, FormatBeginTrans);
    FORMATTER_FACTORY_ENTRY(query, SqliteCommitTrans, FormatCommitTrans);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateVirtualTable, FormatCreateVirtualTable);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateIndex, FormatCreateIndex);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateTrigger, FormatCreateTrigger);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateTrigger::Event, FormatCreateTriggerEvent);
    FORMATTER_FACTORY_ENTRY(query, SqliteCreateView, FormatCreateView);
    FORMATTER_FACTORY_ENTRY(query, SqliteUpdate, FormatUpdate);
    FORMATTER_FACTORY_ENTRY(query, SqliteDelete, FormatDelete);
    FORMATTER_FACTORY_ENTRY(query, SqliteDropIndex, FormatDropIndex);
    FORMATTER_FACTORY_ENTRY(query, SqliteDropTable, FormatDropTable);
    FORMATTER_FACTORY_ENTRY(query, SqliteDropTrigger, FormatDropTrigger);
    FORMATTER_FACTORY_ENTRY(query, SqliteDropView, FormatDropView);
    FORMATTER_FACTORY_ENTRY(query, SqliteOrderBy, FormatOrderBy);
    FORMATTER_FACTORY_ENTRY(query, SqlitePragma, FormatPragma);
    FORMATTER_FACTORY_ENTRY(query, SqliteWindowDefinition, FormatWindowDefinition);
    FORMATTER_FACTORY_ENTRY(query, SqliteWindowDefinition::Window, FormatWindowDefinitionWindow);
    FORMATTER_FACTORY_ENTRY(query, SqliteWindowDefinition::Window::Frame, FormatWindowDefinitionWindowFrame);
    FORMATTER_FACTORY_ENTRY(query, SqliteWindowDefinition::Window::Frame::Bound, FormatWindowDefinitionWindowFrameBound);
    FORMATTER_FACTORY_ENTRY(query, SqliteFilterOver, FormatFilterOver);
    FORMATTER_FACTORY_ENTRY(query, SqliteFilterOver::Filter, FormatFilterOverFilter);
    FORMATTER_FACTORY_ENTRY(query, SqliteFilterOver::Over, FormatFilterOverOver);

    if (!stmt)
    {
        if (query)
            qWarning() << "Unhandled query passed to enterprise formatter!";
        else
            qWarning() << "Null query passed to enterprise formatter!";
    }

    return stmt;
}

void FormatStatement::resetInternal()
{
}

FormatStatement& FormatStatement::withKeyword(const QString& kw)
{
    withToken(FormatToken::KEYWORD, kw);
    return *this;
}

FormatStatement& FormatStatement::withLinedUpKeyword(const QString& kw, const QString& lineUpName)
{
    withToken(FormatToken::LINED_UP_KEYWORD, kw, getFinalLineUpName(lineUpName));
    return *this;
}

FormatStatement& FormatStatement::withId(const QString& id)
{
    withToken(FormatToken::ID, id);
    return *this;
}

FormatStatement& FormatStatement::withId(const QString& id, bool wrapIfNeeded)
{
    withToken(wrapIfNeeded ? FormatToken::ID : FormatToken::ID_NO_WRAP, id);
    return *this;
}

FormatStatement& FormatStatement::withOperator(const QString& oper, FormatToken::Flags flags)
{
    withToken(FormatToken::OPERATOR, oper, flags);
    return *this;
}

FormatStatement&FormatStatement::withStringOrId(const QString& id)
{
    withToken(FormatToken::STRING_OR_ID, id);
    return *this;
}

FormatStatement& FormatStatement::withIdDot(FormatToken::Flags flags)
{
    withToken(FormatToken::ID_DOT, ".", flags);
    return *this;
}

FormatStatement& FormatStatement::withStar(FormatToken::Flags flags)
{
    withToken(FormatToken::STAR, "*", flags);
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

FormatStatement& FormatStatement::withParDefLeft(FormatToken::Flags flags)
{
    withToken(FormatToken::PAR_DEF_LEFT, "(", flags);
    return *this;
}

FormatStatement& FormatStatement::withParDefRight(FormatToken::Flags flags)
{
    withToken(FormatToken::PAR_DEF_RIGHT, ")", flags);
    return *this;
}

FormatStatement& FormatStatement::withParExprLeft(FormatToken::Flags flags)
{
    withToken(FormatToken::PAR_EXPR_LEFT, "(", flags);
    return *this;
}

FormatStatement& FormatStatement::withParExprRight(FormatToken::Flags flags)
{
    withToken(FormatToken::PAR_EXPR_RIGHT, ")", flags);
    return *this;
}

FormatStatement& FormatStatement::withParFuncLeft(FormatToken::Flags flags)
{
    withToken(FormatToken::PAR_FUNC_LEFT, "(", flags);
    return *this;
}

FormatStatement& FormatStatement::withParFuncRight(FormatToken::Flags flags)
{
    withToken(FormatToken::PAR_FUNC_RIGHT, ")", flags);
    return *this;
}

FormatStatement& FormatStatement::withSemicolon(FormatToken::Flags flags)
{
    FormatToken* lastRealToken = getLastRealToken();
    if ((lastRealToken && lastRealToken->type != FormatToken::SEMICOLON) || tokens.size() == 0)
        withToken(FormatToken::SEMICOLON, ";", flags);

    return *this;
}

FormatStatement& FormatStatement::withListComma(FormatToken::Flags flags)
{
    withToken(FormatToken::COMMA_LIST, ",", flags);
    return *this;
}

FormatStatement& FormatStatement::withCommaOper(FormatToken::Flags flags)
{
    withToken(FormatToken::COMMA_OPER, ",", flags);
    return *this;
}

FormatStatement&FormatStatement::withSortOrder(SqliteSortOrder sortOrder)
{
    if (sortOrder != SqliteSortOrder::null)
        withKeyword(sqliteSortOrder(sortOrder));

    return *this;
}

FormatStatement&FormatStatement::withConflict(SqliteConflictAlgo onConflict)
{
    if (onConflict != SqliteConflictAlgo::null)
        withKeyword("ON").withKeyword("CONFLICT").withKeyword(sqliteConflictAlgo(onConflict));

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
    withToken(FormatToken::NEW_LINE, NEWLINE);
    return *this;
}

FormatStatement& FormatStatement::withLiteral(const QVariant& value)
{
    if (value.isNull())
        return *this;

    if (value.userType() == QMetaType::QString)
    {
        withString(value.toString());
        return *this;
    }

    if (value.userType() == QMetaType::QByteArray)
    {
        static_qstring(blobLiteral, "X'%1'");
        withBlob(blobLiteral.arg(QString::fromLatin1(value.toByteArray().toHex())));
        return *this;
    }

    bool ok;
    if (value.userType() == QMetaType::Double)
    {
        value.toDouble(&ok);
        if (ok)
        {
            withFloat(value.toDouble());
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

FormatStatement& FormatStatement::withStatement(SqliteStatement* stmt, const QString& indentName, FormatStatementEnricher enricher)
{
    if (!stmt)
        return *this;

    FormatStatement* formatStmt = forQuery(stmt, wrapper, cfg);
    if (!formatStmt)
        return *this;

    formatStmt->parentFormatStatement = this;

    if (enricher)
        enricher(formatStmt);

    formatStmt->buildTokens();
    formatStmt->deleteTokens = false;

    if (!indentName.isNull())
        markAndKeepIndent(indentName);

    tokens += formatStmt->tokens;

    if (!indentName.isNull())
        withDecrIndent();

    delete formatStmt;
    return *this;
}

FormatStatement& FormatStatement::markIndent(const QString& name)
{
    withToken(FormatToken::INDENT_MARKER, statementName + "_" + name);
    return *this;
}

FormatStatement& FormatStatement::markAndKeepIndent(const QString& name)
{
    markIndent(name);
    withIncrIndent(name);
    return *this;
}

FormatStatement&FormatStatement::withIncrIndent(int newIndent)
{
    withToken(FormatToken::SET_INDENT, newIndent);
    return *this;
}

FormatStatement& FormatStatement::withIncrIndent(const QString& name)
{
    if (name.isNull())
        withToken(FormatToken::INCR_INDENT, name);
    else
        withToken(FormatToken::INCR_INDENT, statementName + "_" + name);

    return *this;
}

FormatStatement& FormatStatement::withDecrIndent()
{
    withToken(FormatToken::DECR_INDENT, QString());
    return *this;
}

FormatStatement&FormatStatement::markKeywordLineUp(const QString& keyword, const QString& lineUpName)
{
    withToken(FormatToken::MARK_KEYWORD_LINEUP, getFinalLineUpName(lineUpName), keyword.length());
    return *this;
}

FormatStatement&FormatStatement::withSeparator(FormatStatement::ListSeparator sep, FormatToken::Flags flags)
{
    switch (sep)
    {
        case ListSeparator::COMMA:
            withListComma(flags);
            break;
        case ListSeparator::EXPR_COMMA:
            withCommaOper(flags);
            break;
        case ListSeparator::NEW_LINE:
            withNewLine();
            break;
        case ListSeparator::SEMICOLON:
            withSemicolon(flags);
            break;
        case ListSeparator::NONE:
            break;
    }
    return *this;
}

void FormatStatement::handleExplainQuery(SqliteQuery* query)
{
    if (query->explain)
    {
        withKeyword("EXPLAIN");
        if (query->queryPlan)
            withKeyword("QUERY").withKeyword("PLAN").withNewLine();
    }
}

FormatStatement& FormatStatement::withIdList(const QStringList& names, const QString& indentName, ListSeparator sep)
{
    if (!indentName.isNull())
        markAndKeepIndent(indentName);

    bool first = true;
    for (const QString& name : names)
    {
        if (!first)
            withSeparator(sep);

        withId(name);
        first = false;
    }

    if (!indentName.isNull())
        withDecrIndent();

    return *this;
}

FormatStatement::FormatToken* FormatStatement::withToken(FormatStatement::FormatToken::Type type, const QVariant& value, const QVariant& additionalValue, FormatToken::Flags flags)
{
    FormatToken* token = new FormatToken;
    token->type = type;
    token->value = value;
    token->additionalValue = additionalValue;
    token->flags = flags;
    tokens << token;
    return token;
}

FormatStatement::FormatToken* FormatStatement::withToken(FormatStatement::FormatToken::Type type, const QVariant& value, FormatToken::Flags flags)
{
    return withToken(type, value, QVariant(), flags);
}

void FormatStatement::cleanup()
{
    kwLineUpPosition.clear();
    line = "";
    lines.clear();
    namedIndents.clear();
    resetIndents();
    if (deleteTokens)
    {
        for (FormatToken* token : tokens)
            delete token;
    }

    tokens.clear();
}

int FormatStatement::getLineUpValue(const QString& lineUpName)
{
    if (kwLineUpPosition.contains(lineUpName))
        return kwLineUpPosition[lineUpName];

    return 0;
}

QString FormatStatement::detokenize()
{
    bool uppercaseKeywords = cfg->SqlEnterpriseFormatter.UppercaseKeywords.get();

    for (FormatToken* token : tokens)
    {
        applySpace(token->type);
        switch (token->type)
        {
            case FormatToken::LINED_UP_KEYWORD:
            {
                if (cfg->SqlEnterpriseFormatter.LineUpKeywords.get())
                {
                    QString kw = token->value.toString();
                    QString lineUpName = token->additionalValue.toString();
                    int lineUpValue = getLineUpValue(lineUpName);

                    int indentLength = lineUpValue - kw.length();
                    if (indentLength > 0)
                        line += SPACE.repeated(indentLength);

                    line += uppercaseKeywords ? kw.toUpper() : kw.toLower();

                    break;
                }
                else
                {
                    // No 'break', so we go to next case, the regular KEYWORD
                }
                Q_FALLTHROUGH();
            }
            case FormatToken::KEYWORD:
            {
                applyIndent();
                line += uppercaseKeywords ? token->value.toString().toUpper() : token->value.toString().toLower();
                break;
            }
            case FormatToken::FUNC_ID:
            case FormatToken::DATA_TYPE:
            {
                applyIndent();
                line += wrapObjIfNeeded(token->value.toString(), wrapper);
                break;
            }
            case FormatToken::ID:
            {
                applyIndent();
                formatId(token->value.toString(), true);
                break;
            }
            case FormatToken::ID_NO_WRAP:
            {
                applyIndent();
                formatId(token->value.toString(), false);
                break;
            }
            case FormatToken::STRING_OR_ID:
            {
                applyIndent();
                QString val = token->value.toString();
                if (val.contains("\""))
                    formatId(token->value.toString(), true);
                else
                    line += wrapObjName(token->value.toString(), NameWrapper::DOUBLE_QUOTE);

                break;
            }
            case FormatToken::STRING:
            {
                applyIndent();
                line += wrapString(token->value.toString());
                break;
            }
            case FormatToken::BLOB:
            case FormatToken::BIND_PARAM:
            case FormatToken::STAR:
            case FormatToken::INTEGER:
            {
                applyIndent();
                line += token->value.toString();
                break;
            }
            case FormatToken::FLOAT:
            {
                applyIndent();
                line += doubleToString(token->value);
                break;
            }
            case FormatToken::OPERATOR:
            {
                bool spaceAdded = endsWithSpace() || applyIndent();
                if (cfg->SqlEnterpriseFormatter.SpaceBeforeMathOp.get() && !spaceAdded && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE))
                    line += SPACE;

                line += token->value.toString();
                if (cfg->SqlEnterpriseFormatter.SpaceAfterMathOp.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER))
                    line += SPACE;

                break;
            }
            case FormatToken::ID_DOT:
            {
                bool spaceAdded = endsWithSpace() || applyIndent();
                if (cfg->SqlEnterpriseFormatter.SpaceBeforeDot.get() && !spaceAdded && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE))
                    line += SPACE;

                line += token->value.toString();
                if (cfg->SqlEnterpriseFormatter.SpaceAfterDot.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER))
                    line += SPACE;

                break;
            }
            case FormatToken::PAR_DEF_LEFT:
            {
                bool spaceBefore = cfg->SqlEnterpriseFormatter.SpaceBeforeOpenPar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE);
                bool spaceAfter = cfg->SqlEnterpriseFormatter.SpaceAfterOpenPar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER);
                bool nlBefore = cfg->SqlEnterpriseFormatter.NlBeforeOpenParDef.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE);
                bool nlAfter = cfg->SqlEnterpriseFormatter.NlAfterOpenParDef.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER);
                detokenizeLeftPar(token, spaceBefore, spaceAfter, nlBefore, nlAfter);
                break;
            }
            case FormatToken::PAR_DEF_RIGHT:
            {
                bool spaceBefore = cfg->SqlEnterpriseFormatter.SpaceBeforeClosePar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE);
                bool spaceAfter = cfg->SqlEnterpriseFormatter.SpaceAfterClosePar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER);
                bool nlBefore = cfg->SqlEnterpriseFormatter.NlBeforeCloseParDef.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE);
                bool nlAfter = cfg->SqlEnterpriseFormatter.NlAfterCloseParDef.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER);
                detokenizeRightPar(token, spaceBefore, spaceAfter, nlBefore, nlAfter);
                break;
            }
            case FormatToken::PAR_EXPR_LEFT:
            {
                bool spaceBefore = cfg->SqlEnterpriseFormatter.SpaceBeforeOpenPar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE);
                bool spaceAfter = cfg->SqlEnterpriseFormatter.SpaceAfterOpenPar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER);
                bool nlBefore = cfg->SqlEnterpriseFormatter.NlBeforeOpenParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE);
                bool nlAfter = cfg->SqlEnterpriseFormatter.NlAfterOpenParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER);
                detokenizeLeftPar(token, spaceBefore, spaceAfter, nlBefore, nlAfter);
                break;
            }
            case FormatToken::PAR_EXPR_RIGHT:
            {
                bool spaceBefore = cfg->SqlEnterpriseFormatter.SpaceBeforeClosePar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE);
                bool spaceAfter = cfg->SqlEnterpriseFormatter.SpaceAfterClosePar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER);
                bool nlBefore = cfg->SqlEnterpriseFormatter.NlBeforeCloseParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE);
                bool nlAfter = cfg->SqlEnterpriseFormatter.NlAfterCloseParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER);
                detokenizeRightPar(token, spaceBefore, spaceAfter, nlBefore, nlAfter);
                break;
            }
            case FormatToken::PAR_FUNC_LEFT:
            {
                bool spaceBefore = cfg->SqlEnterpriseFormatter.SpaceBeforeOpenPar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE) && !cfg->SqlEnterpriseFormatter.NoSpaceAfterFunctionName.get();
                bool spaceAfter = cfg->SqlEnterpriseFormatter.SpaceAfterOpenPar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER);
                bool nlBefore = cfg->SqlEnterpriseFormatter.NlBeforeOpenParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE);
                bool nlAfter = cfg->SqlEnterpriseFormatter.NlAfterOpenParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER);
                detokenizeLeftPar(token, spaceBefore, spaceAfter, nlBefore, nlAfter);
                break;
            }
            case FormatToken::PAR_FUNC_RIGHT:
            {
                bool spaceBefore = cfg->SqlEnterpriseFormatter.SpaceBeforeClosePar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_BEFORE);
                bool spaceAfter = cfg->SqlEnterpriseFormatter.SpaceAfterClosePar.get() && !token->flags.testFlag(FormatToken::Flag::NO_SPACE_AFTER);
                bool nlBefore = cfg->SqlEnterpriseFormatter.NlBeforeCloseParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE);
                bool nlAfter = cfg->SqlEnterpriseFormatter.NlAfterCloseParExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER);
                detokenizeRightPar(token, spaceBefore, spaceAfter, nlBefore, nlAfter);
                break;
            }
            case FormatToken::SEMICOLON:
            {
                if (cfg->SqlEnterpriseFormatter.SpaceNeverBeforeSemicolon.get())
                {
                    removeAllSpaces();
                }
                else
                {
                    bool spaceAdded = endsWithSpace() || applyIndent();
                    if (cfg->SqlEnterpriseFormatter.SpaceBeforeMathOp.get() && !spaceAdded)
                        line += SPACE;
                }

                line += token->value.toString();
                if (cfg->SqlEnterpriseFormatter.NlAfterSemicolon.get())
                    newLine();
                else if (cfg->SqlEnterpriseFormatter.SpaceAfterMathOp.get())
                    line += SPACE;

                break;
            }
            case FormatToken::COMMA_LIST:
            {
                if (cfg->SqlEnterpriseFormatter.SpaceNeverBeforeComma.get() || token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_BEFORE))
                {
                    removeAllSpaces();
                }
                else
                {
                    bool spaceAdded = endsWithSpace() || applyIndent();
                    if (cfg->SqlEnterpriseFormatter.SpaceBeforeCommaInList.get() && !spaceAdded)
                        line += SPACE;
                }

                line += token->value.toString();
                if (cfg->SqlEnterpriseFormatter.NlAfterComma.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER))
                    newLine();
                else if (cfg->SqlEnterpriseFormatter.SpaceAfterCommaInList.get())
                    line += SPACE;

                break;
            }
            case FormatToken::COMMA_OPER:
            {
                if (cfg->SqlEnterpriseFormatter.SpaceNeverBeforeComma.get())
                {
                    removeAllSpaces();
                }
                else
                {
                    bool spaceAdded = endsWithSpace() || applyIndent();
                    if (cfg->SqlEnterpriseFormatter.SpaceBeforeCommaInList.get() && !spaceAdded)
                        line += SPACE;
                }

                line += token->value.toString();
                if (cfg->SqlEnterpriseFormatter.NlAfterCommaInExpr.get() && !token->flags.testFlag(FormatToken::Flag::NO_NEWLINE_AFTER))
                    newLine();
                else if (cfg->SqlEnterpriseFormatter.SpaceAfterCommaInList.get())
                    line += SPACE;

                break;
            }
            case FormatToken::NEW_LINE:
            {
                newLine();
                break;
            }
            case FormatToken::INDENT_MARKER:
            {
                QString indentName = token->value.toString();
                namedIndents[indentName] = predictCurrentIndent(token);
                break;
            }
            case FormatToken::INCR_INDENT:
            {
                if (!token->value.isNull())
                    incrIndent(token->value.toString());
                else
                    incrIndent();

                break;
            }
            case FormatToken::SET_INDENT:
            {
                setIndent(indents.top() + token->value.toInt());
                break;
            }
            case FormatToken::DECR_INDENT:
            {
                decrIndent();
                break;
            }
            case FormatToken::MARK_KEYWORD_LINEUP:
            {
                QString lineUpName = token->value.toString();
                int lineUpLength = predictCurrentIndent(token) + token->additionalValue.toInt();
                if (!kwLineUpPosition.contains(lineUpName) || lineUpLength > kwLineUpPosition[lineUpName])
                    kwLineUpPosition[lineUpName] = lineUpLength;

                break;
            }
        }
        updateLastToken(token);
    }
    newLine();
    return lines.join(NEWLINE);
}

bool FormatStatement::applyIndent()
{
    int indentToAdd = indents.top() - line.length();
    if (indentToAdd <= 0)
        return false;

    line += SPACE.repeated(indentToAdd);
    return true;
}

void FormatStatement::applySpace(FormatToken::Type type)
{
    if (lastToken && isSpaceExpectingType(type) && isSpaceExpectingType(lastToken->type) && !endsWithSpace())
        line += SPACE;
}

bool FormatStatement::isSpaceExpectingType(FormatStatement::FormatToken::Type type)
{
    switch (type)
    {
        case FormatToken::KEYWORD:
        case FormatToken::LINED_UP_KEYWORD:
        case FormatToken::ID:
        case FormatToken::ID_NO_WRAP:
        case FormatToken::STRING_OR_ID:
        case FormatToken::FLOAT:
        case FormatToken::STRING:
        case FormatToken::INTEGER:
        case FormatToken::BLOB:
        case FormatToken::BIND_PARAM:
        case FormatToken::FUNC_ID:
        case FormatToken::DATA_TYPE:
        case FormatToken::STAR:
            return true;
        case FormatToken::OPERATOR:
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
        case FormatToken::SET_INDENT:
        case FormatToken::MARK_KEYWORD_LINEUP:
            break;
    }
    return false;
}

bool FormatStatement::isMetaType(FormatStatement::FormatToken::Type type)
{
    switch (type)
    {
        case FormatToken::INDENT_MARKER:
        case FormatToken::INCR_INDENT:
        case FormatToken::DECR_INDENT:
        case FormatToken::SET_INDENT:
        case FormatToken::MARK_KEYWORD_LINEUP:
            return true;
        case FormatToken::KEYWORD:
        case FormatToken::LINED_UP_KEYWORD:
        case FormatToken::ID:
        case FormatToken::ID_NO_WRAP:
        case FormatToken::STRING_OR_ID:
        case FormatToken::FLOAT:
        case FormatToken::STRING:
        case FormatToken::INTEGER:
        case FormatToken::BLOB:
        case FormatToken::BIND_PARAM:
        case FormatToken::FUNC_ID:
        case FormatToken::DATA_TYPE:
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
            break;
    }
    return false;
}

void FormatStatement::newLine()
{
    if (line.length() == 0) // prevents double new-line when for example "))" occurs and it has new-line before and after
        return;

    lines << line;
    line = "";
}

void FormatStatement::incrIndent(const QString& name)
{
    if (!name.isNull())
    {
        if (namedIndents.contains(name))
        {
            indents.push(namedIndents[name]);
        }
        else
        {
            indents.push(indents.top() + cfg->SqlEnterpriseFormatter.TabSize.get());
            qCritical() << __func__ << "No named indent found:" << name;
        }
    }
    else
        indents.push(indents.top() + cfg->SqlEnterpriseFormatter.TabSize.get());
}

void FormatStatement::decrIndent()
{
    if (indents.size() <= 1)
        return;

    indents.pop();
}

void FormatStatement::setIndent(int newIndent)
{
    indents.push(newIndent);
}

bool FormatStatement::endsWithSpace()
{
    return line.length() == 0 || line[line.length() - 1].isSpace();
}

FormatStatement::FormatToken* FormatStatement::getLastRealToken(bool skipNewLines)
{
    for (FormatToken* tk : reverse(tokens))
    {
        if (!isMetaType(tk->type) && (!skipNewLines || tk->type != FormatToken::NEW_LINE))
            return tk;
    }
    return nullptr;
}

FormatStatement::FormatToken* FormatStatement::getLastToken()
{
    return tokens.last();
}

void FormatStatement::detokenizeLeftPar(FormatToken* token, bool spaceBefore, bool spaceAfter, bool nlBefore, bool nlAfter)
{
    bool spaceAdded = endsWithSpace();
    if (nlBefore)
    {
        newLine();
        spaceAdded = true;
    }

    spaceAdded |= applyIndent();
    if (spaceBefore && !spaceAdded)
        line += SPACE;

    line += token->value.toString();
    if (nlAfter)
    {
        newLine();
        if (cfg->SqlEnterpriseFormatter.IndentParenthesisBlock.get())
            incrIndent();
    }
    else if (spaceAfter)
        line += SPACE;
}

void FormatStatement::detokenizeRightPar(FormatStatement::FormatToken* token, bool spaceBefore, bool spaceAfter, bool nlBefore, bool nlAfter)
{
    bool spaceAdded = endsWithSpace();
    if (nlBefore)
    {
        newLine();
        spaceAdded = true;
        if (cfg->SqlEnterpriseFormatter.IndentParenthesisBlock.get())
            decrIndent();
    }

    spaceAdded |= applyIndent();
    if (spaceBefore && !spaceAdded)
        line += SPACE;

    line += token->value.toString();
    if (nlAfter)
        newLine();
    else if (spaceAfter)
        line += SPACE;
}

void FormatStatement::resetIndents()
{
    indents.clear();
    indents.push(0);
}

void FormatStatement::removeAllSpaces()
{
    removeAllSpacesFromLine();
    while (endsWithSpace() && lines.size() > 0)
    {
        line = lines.takeLast();
        removeAllSpacesFromLine();

        if (lines.size() == 0)
            break;
    }
}

void FormatStatement::removeAllSpacesFromLine()
{
    while (endsWithSpace() && line.length() > 0)
        line.chop(1);
}

void FormatStatement::updateLastToken(FormatStatement::FormatToken* token)
{
    if (!isMetaType(token->type))
        lastToken = token;
}

QString FormatStatement::getFinalLineUpName(const QString& lineUpName)
{
    QString finalName = statementName;
    if (!lineUpName.isNull())
        finalName += "_" + lineUpName;

    return finalName;
}

int FormatStatement::predictCurrentIndent(FormatToken* currentMetaToken)
{
    QString lineBackup = line;
    bool isSpace = applyIndent() || endsWithSpace();

    if (!isSpace)
    {
        // We haven't added any space and there is no space currently at the end of line.
        // We need to predict if next real (printable) token will require space to be added.
        // If yes, we add it virtually here, so we know the indent required afterwards.
        // First we need to find next real token:
        int tokenIdx = tokens.indexOf(currentMetaToken);
        FormatToken* nextRealToken = nullptr;
        for (FormatToken* tk : tokens.mid(tokenIdx + 1))
        {
            if (!isMetaType(tk->type))
            {
                nextRealToken = tk;
                break;
            }
        }

        // If the real token was found we can see if it will require additional space for indent:
        if ((nextRealToken && isSpaceExpectingType(lastToken->type) && isSpaceExpectingType(nextRealToken->type)) || willStartWithNewLine(nextRealToken))
        {
            // Next real token does not start with new line, but it does require additional space:
            line += SPACE;
        }
    }

    int result = line.length();
    line = lineBackup;
    return result;
}

bool FormatStatement::willStartWithNewLine(FormatStatement::FormatToken* token)
{
    return (token->type == FormatToken::PAR_DEF_LEFT && cfg->SqlEnterpriseFormatter.NlBeforeOpenParDef) ||
            (token->type == FormatToken::PAR_EXPR_LEFT && cfg->SqlEnterpriseFormatter.NlBeforeOpenParExpr) ||
            (token->type == FormatToken::PAR_FUNC_LEFT && cfg->SqlEnterpriseFormatter.NlBeforeOpenParExpr) ||
            (token->type == FormatToken::PAR_DEF_RIGHT && cfg->SqlEnterpriseFormatter.NlBeforeCloseParDef) ||
            (token->type == FormatToken::PAR_EXPR_RIGHT && cfg->SqlEnterpriseFormatter.NlBeforeCloseParExpr) ||
            (token->type == FormatToken::PAR_FUNC_RIGHT && cfg->SqlEnterpriseFormatter.NlBeforeCloseParExpr) ||
            (token->type == FormatToken::NEW_LINE);
}

void FormatStatement::formatId(const QString& value, bool applyWrapping)
{
    if (!applyWrapping)
    {
        line += value;
        return;
    }

    if (cfg->SqlEnterpriseFormatter.AlwaysUseNameWrapping.get())
        line += wrapObjName(value, true, wrapper);
    else
        line += wrapObjIfNeeded(value, true, wrapper);
}

FormatStatement* FormatStatement::forQuery(SqliteStatement* query, NameWrapper wrapper, Cfg::SqlEnterpriseFormatterConfig* cfg)
{
    FormatStatement* formatStmt = forQuery(query);
    if (formatStmt)
    {
        formatStmt->wrapper = wrapper;
        formatStmt->cfg = cfg;
    }
    return formatStmt;
}
