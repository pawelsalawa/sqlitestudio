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
}

FormatStatement::~FormatStatement()
{
}

QString FormatStatement::format()
{
    kwLineUpPosition = 0;
    lines.clear();
    line = "";

    resetInternal();
    formatInternal();

    lines << line;
    return lines.join("\n");
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

void FormatStatement::keywordToLineUp(const QString &keyword)
{
    int lgt = keyword.length();
    if (lgt >= kwLineUpPosition)
        kwLineUpPosition = lgt;
}

void FormatStatement::pushIndent()
{
    indents.push(line.length());
}

void FormatStatement::popIndent()
{
    indents.pop();
}

void FormatStatement::newLine()
{
    lines << line;
    line = "";
}

void FormatStatement::append(const QString &str)
{
    if (str.contains("\n"))
    {
        QStringList localLines = str.split("\n");
        QString localIndent;
        if (localLines.first() == "\n")
        {
            QString tmpLine = line;
            tmpLine.remove(QRegularExpression("\\S+\\s*^"));
            localIndent = SPACE.repeated(tmpLine.length());
        }
        else
        {
            localIndent = SPACE.repeated(line.length());
        }

        QMutableStringListIterator it(localLines);
        line += it.next();
        while (it.hasNext())
        {
            newLine();
            line += it.next().prepend(localIndent);
        }
    }
    else
        line.append(str);
}

void FormatStatement::appendKeyword(const QString &str)
{
    QString nextStr = prepareNextStr(str);
    if (CFG_ADV_FMT.SqlEnterpriseFormatter.UppercaseKeywords.get())
        line += nextStr.toUpper();
    else
        line += nextStr.toLower();
}

void FormatStatement::appendDataType(const QString &str)
{
    QString nextStr = prepareNextStr(str);
    if (CFG_ADV_FMT.SqlEnterpriseFormatter.UppercaseDataTypes.get())
        line += nextStr.toUpper();
    else
        line += nextStr;
}

void FormatStatement::appendLeftExprPar()
{
    appendPar("(",
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlBeforeOpenParExpr.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeOpenPar.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlAfterOpenParExpr.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterOpenPar.get());
}

void FormatStatement::appendRightExprPar()
{
    appendPar(")",
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlBeforeCloseParExpr.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeClosePar.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlAfterCloseParExpr.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterClosePar.get());
}

void FormatStatement::appendLeftDefPar()
{
    appendPar("(",
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlBeforeOpenParDef.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeOpenPar.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlAfterOpenParDef.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterOpenPar.get());
}

void FormatStatement::appendRightDefPar()
{
    appendPar(")",
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlBeforeCloseParDef.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeClosePar.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.NlAfterCloseParDef.get(),
              CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterClosePar.get());
}

void FormatStatement::appendName(const QString &str)
{
    QString wrapped;
    if (CFG_ADV_FMT.SqlEnterpriseFormatter.AlwaysUseNameWrapping.get())
        wrapped = wrapObjName(str, dialect, wrapper);
    else
        wrapped = wrapObjIfNeeded(str, dialect, wrapper);

    line += wrapped;
}

void FormatStatement::appendLinedUpKeyword(const QString &str)
{
    int spaces = kwLineUpPosition - str.length();
    if (spaces > 0)
        line.append(SPACE.repeated(spaces));

    appendKeyword(str);
}

void FormatStatement::appendLinedUpPrefixedString(const QString &prefixWord, const QString &str)
{
    int spaces = kwLineUpPosition - prefixWord.length();
    if (spaces > 0)
        line.append(SPACE.repeated(spaces));

    line.append(str);
}

void FormatStatement::appendNameDot()
{
    trimLineEnd();

    QString str = ".";
    if (CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeDot.get())
        str.prepend(" ");

    if (CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterDot.get())
        str.append(" ");

    line += str;
}

QString FormatStatement::format(SqliteStatement* stmt)
{
    FormatStatement* formatStmt = forQuery(stmt);
    if (!formatStmt)
        return query->detokenize();

    formatStmt->wrapper = wrapper;
    QString result = formatStmt->format();
    delete formatStmt;

    return result;
}

QString FormatStatement::formatExprList(const QList<SqliteStatement *> &stmts)
{
    return formatStmtList(stmts, CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeCommaInList.get(),
                          CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterCommaInList.get(),
                          CFG_ADV_FMT.SqlEnterpriseFormatter.NlAfterCommaInExpr.get());
}

QString FormatStatement::formatDefList(const QList<SqliteStatement *> &stmts)
{
    return formatStmtList(stmts, CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceBeforeCommaInList.get(),
                          CFG_ADV_FMT.SqlEnterpriseFormatter.SpaceAfterCommaInList.get(),
                          CFG_ADV_FMT.SqlEnterpriseFormatter.NlAfterComma.get());
}

QString FormatStatement::formatStmtList(const QList<SqliteStatement *> &stmts, bool spaceBeforeSep, bool spaceAfterSep, bool nlAfterSep)
{
    QStringList entries;
    for (SqliteStatement* stmt : stmts)
        entries << format(stmt);

    QString sep = ",";
    if (spaceBeforeSep)
        sep.prepend(" ");

    if (nlAfterSep)
        sep.append("\n");
    else if (spaceAfterSep)
        sep.append(" ");

    return entries.join(sep);
}

void FormatStatement::appendPar(const QString& par, bool nlBefore, bool spaceBefore, bool nlAfter, bool spaceAfter)
{
    trimLineEnd();

    QString str = par;
    if (nlBefore)
        str.prepend("\n");
    else if (spaceBefore)
        str.prepend(" ");

    if (nlAfter)
        str.append("\n");
    else if (spaceAfter)
        str.append(" ");

    append(str);
}

void FormatStatement::trimLineEnd()
{
    line.remove(QRegularExpression("\\s*$"));
}
