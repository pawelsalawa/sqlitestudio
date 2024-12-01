#include "htmlexport.h"
#include "services/pluginmanager.h"
#include "schemaresolver.h"
#include "common/unused.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QStringEncoder>

QString HtmlExport::getFormatName() const
{
    return "HTML";
}

ExportManager::StandardConfigFlags HtmlExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

QString HtmlExport::getExportConfigFormName() const
{
    return "HtmlExportConfig";
}

void HtmlExport::validateOptions()
{
    bool header = cfg.HtmlExport.PrintHeader.get();
    EXPORT_MANAGER->updateVisibilityAndEnabled(cfg.HtmlExport.PrintDataTypes, true, header);
}

QString HtmlExport::defaultFileExtension() const
{
    return "html";
}

CfgMain* HtmlExport::getConfig()
{
    return &cfg;
}

bool HtmlExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(query);
    UNUSED(providedData);

    if (!beginDoc(tr("SQL query results")))
        return false;

    columnTypes = QueryExecutor::resolveColumnTypes(db, columns, true);

    writeln("<table>");
    incrIndent();
    if (printHeader)
    {
        writeln("<tr class=\"header\">");
        incrIndent();
        if (printRownum)
        {
            writeln("<td align=\"right\">");
            incrIndent();
            writeln("<b><i>#</i></b>");
            decrIndent();
            writeln("</td>");
        }

        QString column;
        int i = 0;
        for (const QueryExecutor::ResultColumnPtr& col : columns)
        {
            writeln("<td>");
            incrIndent();
            column = QString("<b>%1</b>").arg(col->displayName);
            if (printDatatypes)
            {
                if (!columnTypes[i].isNull())
                    column.append("<br/>" + columnTypes[i].toFullTypeString());
                else
                    column.append("<br/>" + tr("no type"));
            }
            writeln(column);
            decrIndent();
            writeln("</td>");
            i++;
        }
        decrIndent();
        writeln("</tr>");
    }

    currentDataRow = 0;
    return true;
}

bool HtmlExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    return exportDataRow(row);
}

bool HtmlExport::afterExportQueryResults()
{
    decrIndent();
    writeln("</table>");
    writeln("<br/><br/>");
    return true;
}

bool HtmlExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(database);
    UNUSED(ddl);
    UNUSED(columnNames);
    UNUSED(providedData);

    if (isTableExport())
    {
        if (!beginDoc(tr("Exported table: %1").arg(table)))
            return false;
    }

    int colCount = createTable->columns.size();
    int colSpan = printRownum ? colCount + 1 : colCount;

    writeln("<table>");
    incrIndent();

    writeln("<tr class=\"title\">");
    incrIndent();
    writeln(QString("<td colspan=\"%1\" align=\"center\">%2</td>").arg(colSpan).arg(tr("Table: %1").arg(table)));
    decrIndent();
    writeln("</tr>");

    if (printHeader)
    {
        writeln("<tr class=\"header\">");
        incrIndent();
        if (printRownum)
        {
            writeln("<td align=\"right\">");
            incrIndent();
            writeln("<b><i>#</i></b>");
            decrIndent();
            writeln("</td>");
        }

        QString column;
        for (SqliteCreateTable::Column* col : createTable->columns)
        {
            writeln("<td>");
            incrIndent();
            column = QString("<b>%1</b>").arg(col->name);
            if (printDatatypes)
            {
                if (col->type)
                    column.append("<br/>" + col->type->toDataType().toFullTypeString());
                else
                    column.append("<br/>" + tr("no type"));
            }
            writeln(column);
            decrIndent();
            writeln("</td>");
        }
        decrIndent();
        writeln("</tr>");
    }

    columnTypes.clear();
    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        if (col->type)
            columnTypes << col->type->toDataType();
        else
            columnTypes << DataType();
    }

    currentDataRow = 0;
    return true;
}

bool HtmlExport::exportDataRow(SqlResultsRowPtr data)
{
    currentDataRow++;

    writeln("<tr>");
    incrIndent();
    if (printRownum)
    {
        writeln("<td class=\"rownum\">");
        incrIndent();
        writeln(QString("<i>%1</i>").arg(currentDataRow));
        decrIndent();
        writeln("</td>");
    }

    QString align;
    QString cellValue;
    QString cellStyle;
    int i = 0;
    for (const QVariant& value : data->valueList())
    {
        if (columnTypes[i].isNumeric())
            align = "right";
        else
            align = "left";

        if (value.isNull())
        {
            cellValue = "<i>NULL</i>";
            cellStyle = " class=\"null\"";
        }
        else
        {
            cellStyle = "";
            if (value.toString().trimmed().isEmpty())
                cellValue = "&nbsp;";
            else
            {
                cellValue = value.toString();
                cellValue.truncate(byteLengthLimit);
                cellValue = escape(cellValue);
            }
        }
        writeln(QString("<td align=\"%1\"%2>").arg(align, cellStyle));
        incrIndent();
        writeln(cellValue);
        decrIndent();
        writeln("</td>");
        i++;
    }
    decrIndent();
    writeln("</tr>");
    return true;
}

bool HtmlExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(database);
    UNUSED(ddl);
    UNUSED(columnNames);
    UNUSED(createTable);
    UNUSED(providedData);

    if (isTableExport())
    {
        if (!beginDoc(tr("Exported table: %1").arg(table)))
            return false;
    }

    int colCount = columnNames.size();
    int colSpan = printRownum ? colCount + 1 : colCount;

    writeln("<table>");
    incrIndent();

    writeln("<tr class=\"title\">");
    incrIndent();
    writeln(QString("<td colspan=\"%1\" align=\"center\">%2 <font color=\"#777777\">(%3)</font></td>").arg(colSpan).arg(tr("Table: %1").arg(table), tr("virtual")));
    decrIndent();
    writeln("</tr>");

    if (printHeader)
    {
        writeln("<tr class=\"header\">");
        incrIndent();
        if (printRownum)
        {
            writeln("<td align=\"right\">");
            incrIndent();
            writeln("<b><i>#</i></b>");
            decrIndent();
            writeln("</td>");
        }

        QString column;
        for (const QString& col : columnNames)
        {
            writeln("<td>");
            incrIndent();
            writeln(QString("<b>%1</b>").arg(col));
            decrIndent();
            writeln("</td>");
        }
        decrIndent();
        writeln("</tr>");
    }

    columnTypes.clear();
    for (int i = 0, total = columnNames.size(); i < total; ++i)
        columnTypes << DataType();

    currentDataRow = 0;
    return true;
}

bool HtmlExport::exportTableRow(SqlResultsRowPtr data)
{
    return exportDataRow(data);
}

bool HtmlExport::afterExportTable()
{
    decrIndent();
    writeln("</table>");
    writeln("<br/><br/>");
    return true;
}

bool HtmlExport::beforeExportDatabase(const QString& database)
{
    if (!beginDoc(tr("Exported database: %1").arg(database)))
        return false;

    return true;
}

bool HtmlExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    UNUSED(database);
    UNUSED(ddl);

    writeln("<table>");
    incrIndent();

    writeln("<tr class=\"title\">");
    incrIndent();
    writeln(QString("<td align=\"center\" colspan=\"3\">%1</td>").arg(tr("Index: %1").arg("<b>" + name + "</b>")));
    decrIndent();
    writeln("</tr>");

    writeln("<tr>");
    incrIndent();
    writeln(QString("<td align=\"right\" class=\"rownum\">%1</td>").arg(tr("For table:")));
    writeln(QString("<td colspan=\"2\">%1</td>").arg(createIndex->table));
    decrIndent();
    writeln("</tr>");

    writeln("<tr>");
    incrIndent();
    writeln(QString("<td align=\"right\" class=\"rownum\">%1</td>").arg(tr("Unique:")));
    writeln(QString("<td colspan=\"2\">%1</td>").arg(createIndex->uniqueKw ? tr("Yes") : tr("No")));
    decrIndent();
    writeln("</tr>");

    writeln("<tr class=\"header\">");
    incrIndent();
    writeln(QString("<td><b>%1</b></td>").arg(tr("Column")));
    writeln(QString("<td><b>%1</b></td>").arg(tr("Collating")));
    writeln(QString("<td><b>%1</b></td>").arg(tr("Sort order")));
    decrIndent();
    writeln("</tr>");

    QString collate;
    for (SqliteOrderBy* idxCol : createIndex->indexedColumns)
    {
        collate = idxCol->getCollation();

        writeln("<tr>");
        incrIndent();
        writeln(QString("<td>%1</td>").arg(escape(idxCol->getColumnString())));
        writeln(QString("<td align=\"center\">%1</td>").arg(collate.isNull() ? "&nbsp;" : escape(collate)));
        writeln(QString("<td align=\"center\">%1</td>").arg(idxCol->order == SqliteSortOrder::null ? "&nbsp;" : sqliteSortOrder(idxCol->order)));
        decrIndent();
        writeln("</tr>");
    }

    decrIndent();
    writeln("</table>");
    writeln("<br/><br/>");
    return true;
}

bool HtmlExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    UNUSED(database);
    UNUSED(ddl);

    writeln("<table>");
    incrIndent();

    writeln("<tr class=\"title\">");
    incrIndent();
    writeln(QString("<td align=\"center\" colspan=\"2\">%1</td>").arg(tr("Trigger: %1").arg("<b>" + name + "</b>")));
    decrIndent();
    writeln("</tr>");

    writeln("<tr>");
    incrIndent();
    writeln(QString("<td align=\"right\" class=\"rownum\">%1</td>").arg(tr("Activated:")));
    writeln(QString("<td><code>%1</code></td>").arg(SqliteCreateTrigger::time(createTrigger->eventTime)));
    decrIndent();
    writeln("</tr>");

    QString event = createTrigger->event ? SqliteCreateTrigger::Event::typeToString(createTrigger->event->type) : "";
    writeln("<tr>");
    incrIndent();
    writeln(QString("<td align=\"right\" class=\"rownum\">%1</td>").arg(tr("Action:")));
    writeln(QString("<td><code>%1</code></td>").arg(event));
    decrIndent();
    writeln("</tr>");

    QString onObj;
    if (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF)
        onObj = tr("On view:");
    else
        onObj = tr("On table:");

    writeln("<tr>");
    incrIndent();
    writeln(QString("<td align=\"right\" class=\"rownum\">%1</td>").arg(onObj));
    writeln(QString("<td><code>%1</code></td>").arg(createTrigger->table));
    decrIndent();
    writeln("</tr>");

    writeln("<tr>");
    incrIndent();
    writeln(QString("<td align=\"right\" class=\"rownum\">%1</td>").arg(tr("Activate condition:")));
    writeln(QString("<td><code>%1</code></td>").arg(createTrigger->precondition ? escape(createTrigger->precondition->detokenize()) : ""));
    decrIndent();
    writeln("</tr>");

    writeln("<tr>");
    incrIndent();
    writeln(QString("<td colspan=\"2\" class=\"separator\">%1</td>").arg(tr("Code executed:")));
    decrIndent();
    writeln("</tr>");

    QStringList queryStrings;
    for (SqliteQuery* q : createTrigger->queries)
        queryStrings << escape(q->detokenize());

    writeln("<tr>");
    incrIndent();
    writeln("<td colspan=\"2\">");
    incrIndent();
    writeln(QString("<pre>%1</pre>").arg(queryStrings.join("<br/>")));
    decrIndent();
    writeln("</td>");
    decrIndent();
    writeln("</tr>");

    decrIndent();
    writeln("</table>");
    writeln("<br/><br/>");
    return true;
}

bool HtmlExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view)
{
    UNUSED(database);
    UNUSED(ddl);

    writeln("<table>");
    incrIndent();

    writeln("<tr class=\"title\">");
    incrIndent();
    writeln(QString("<td align=\"center\">%1</td>").arg(tr("View: %1").arg("<b>" + name + "</b>")));
    decrIndent();
    writeln("</tr>");

    writeln("<tr>");
    incrIndent();
    writeln("<td>");
    incrIndent();
    writeln(QString("<pre>%1</pre>").arg(escape(view->select->detokenize())));
    decrIndent();
    writeln("</td>");
    decrIndent();
    writeln("</tr>");

    decrIndent();
    writeln("</table>");
    writeln("<br/><br/>");
    return true;
}

bool HtmlExport::afterExport()
{
    static const QString bodyEndTpl = QStringLiteral("</body>");
    static const QString docEnd = QStringLiteral("</html>");

    writeln("<i>" + tr("Document generated by SQLiteStudio v%1 on %2").arg(SQLITESTUDIO->getVersionString(), QDateTime::currentDateTime().toString()) + "</i>");
    decrIndent();
    writeln(bodyEndTpl);
    decrIndent();
    writeln(docEnd);
    return true;
}

bool HtmlExport::beginDoc(const QString& title)
{
    static const QString docStart = QStringLiteral(
                R"(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">)"
                "\n<html>"
                );

    static const QString metaCodecTpl = QStringLiteral(R"(<meta http-equiv="Content-Type" content="text/html; charset=%1"/>)");
    static const QString titletpl = QStringLiteral(R"(<title>%1</title>)");
    static const QString styleStartTpl = QStringLiteral(R"(<style type="text/css">)");
    static const QString styleEndTpl = QStringLiteral(R"(</style>)");
    static const QString bodyStartTpl = QStringLiteral(R"(<body>)");

    setupConfig();

    QFile file(":/htmlexport/htmlexport.css");
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Could not open htmlexport.css resource while exporting to HTML:" << file.errorString();
        return false;
    }

    writeln(docStart);
    incrIndent();
    writeln(metaCodecTpl.arg(codecName));
    writeln(titletpl.arg(title));
    writeln(styleStartTpl);
    incrIndent();
    writeln(indent ? file.readAll() : compressCss(file.readAll()));
    decrIndent();
    writeln(styleEndTpl);
    writeln(bodyStartTpl);
    incrIndent();

    file.close();
    return true;
}

void HtmlExport::setupConfig()
{
    codecName = codec->name();
    indentDepth = 0;
    newLineStr = "";
    indentStr = "";
    indent = (cfg.HtmlExport.Format.get() == "format");
    if (indent)
        newLineStr = "\n";

    printRownum = cfg.HtmlExport.PrintRowNum.get();
    printHeader = cfg.HtmlExport.PrintHeader.get();
    printDatatypes = printHeader && cfg.HtmlExport.PrintDataTypes.get();
    byteLengthLimit = cfg.HtmlExport.ByteLengthLimit.get();
}

void HtmlExport::incrIndent()
{
    if (indent)
    {
        indentDepth++;
        updateIndent();
    }
}

void HtmlExport::decrIndent()
{
    if (indent)
    {
        indentDepth--;
        updateIndent();
    }
}

void HtmlExport::updateIndent()
{
    indentStr = QString("    ").repeated(indentDepth);
}

void HtmlExport::writeln(const QString& str)
{
    QString newStr;
    if (str.contains("\n"))
    {
        QStringList lines = str.split("\n");
        QMutableStringListIterator it(lines);
        while (it.hasNext())
            it.next().prepend(indentStr);

        newStr = lines.join("\n") + newLineStr;
    }
    else
    {
        newStr = indentStr + str + newLineStr;
    }
    GenericExportPlugin::write(newStr);
}

QString HtmlExport::escape(const QString& str)
{
    if (cfg.HtmlExport.DontEscapeHtml.get())
        return str;

    return str.toHtmlEscaped();
}

QString HtmlExport::compressCss(QString css)
{
    static const QRegularExpression spacesLeftRe(R"REGEXP(([^a-zA-Z0-9_\s]+)\s+(\S+))REGEXP");
    static const QRegularExpression spacesRightRe(R"REGEXP((\S+)\s+([^a-zA-Z0-9_\s]+))REGEXP");
    static const QRegularExpression spacesBetweenWordsRe(R"REGEXP((\S+)\s{2,}(\S+))REGEXP");
    while (css.indexOf(spacesLeftRe) > -1)
        css.replace(spacesLeftRe, R"(\1\2)");

    while (css.indexOf(spacesRightRe) > -1)
        css.replace(spacesRightRe, R"(\1\2)");

    while (css.indexOf(spacesBetweenWordsRe) > -1)
        css.replace(spacesBetweenWordsRe, R"(\1 \2)");

    return css.trimmed();
}

bool HtmlExport::init()
{
    SQLS_INIT_RESOURCE(htmlexport);
    return GenericExportPlugin::init();
}

void HtmlExport::deinit()
{
    SQLS_CLEANUP_RESOURCE(htmlexport);
}
