#include "xmlexport.h"
#include "services/exportmanager.h"
#include "common/unused.h"
#include <QTextCodec>

const QString XmlExport::docBegin = QStringLiteral("<?xml version=\"1.0\" encoding=\"%1\"?>\n");

XmlExport::XmlExport()
{
}

QString XmlExport::getFormatName() const
{
    return QStringLiteral("XML");
}

ExportManager::StandardConfigFlags XmlExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

QString XmlExport::getExportConfigFormName() const
{
    return QStringLiteral("XmlExportConfig");
}

CfgMain* XmlExport::getConfig()
{
    return &cfg;
}

void XmlExport::validateOptions()
{
    bool useNs = cfg.XmlExport.UseNamespace.get();
    EXPORT_MANAGER->updateVisibilityAndEnabled(cfg.XmlExport.Namespace, true, useNs);

    bool nsValid = !useNs || !cfg.XmlExport.Namespace.get().isEmpty();
    EXPORT_MANAGER->handleValidationFromPlugin(nsValid, cfg.XmlExport.Namespace, tr("Enter the namespace to use (for example: http://my.namespace.org"));
}

QString XmlExport::defaultFileExtension() const
{
    return QStringLiteral("xml");
}

bool XmlExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(providedData);

    setupConfig();

    write(docBegin.arg(codecName));

    writeln(QString("<results%2>").arg(nsStr));
    incrIndent();

    writeln("<query>");
    incrIndent();
    writeln(escape(query));
    decrIndent();
    writeln("</query>");

    QList<DataType> columnTypes = QueryExecutor::resolveColumnTypes(db, columns, true);
    writeln("<columns>");
    incrIndent();
    int i = 0;
    DataType type;
    for (QueryExecutor::ResultColumnPtr col : columns)
    {
        type = columnTypes[i];

        writeln("<column>");
        incrIndent();
        writeln("<displayName>"+ escape(col->displayName) + "</displayName>");
        writeln("<name>"+ escape(col->column) + "</name>");
        writeln("<table>"+ escape(col->table) + "</table>");
        writeln("<database>"+ escape(col->database) + "</database>");
        writeln("<type>"+ escape(type.toFullTypeString()) + "</type>");
        decrIndent();
        writeln("</column>");
        i++;
    }
    decrIndent();
    writeln("</columns>");

    writeln("<rows>");
    incrIndent();
    return true;
}

bool XmlExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    static const QString rowTpl = QStringLiteral("<value column=\"%1\">%2</value>");
    static const QString nullTpl = QStringLiteral("<value column=\"%1\" null=\"true\"/>");

    writeln("<row>");
    incrIndent();

    int i = 0;
    for (const QVariant& value : row->valueList())
    {
        if (value.isNull())
            writeln(nullTpl.arg(i));
        else
            writeln(rowTpl.arg(i).arg(escape(value.toString())));
    }

    decrIndent();
    writeln("</row>");
    return true;
}

bool XmlExport::afterExportQueryResults()
{
    decrIndent();
    write("</rows>");
    decrIndent();
    write("</results>");
    return true;
}

bool XmlExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(columnNames);
    UNUSED(providedData);
    if (isTableExport())
    {
        setupConfig();
        write(docBegin.arg(codecName));
    }

    writeln(QString("<table%1>").arg(isTableExport() ? nsStr : ""));
    incrIndent();

    writeln("<database>" + escape(database) + "</database>");
    writeln("<name>" + escape(table) + "</name>");
    if (!createTable->withOutRowId.isNull())
        writeln(QString("<withoutRowId>true</withoutRowId>"));

    writeln("<ddl>" + escape(ddl) + "</ddl>");

    writeln("<columns>");
    incrIndent();
    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        writeln("<column>");
        incrIndent();
        writeln("<name>"+ col->name + "</name>");
        writeln(QString("<type>%1</type>").arg((col->type ? col->type->toDataType().toFullTypeString() : "")));
        if (col->constraints.size() > 0)
        {
            writeln("<constraints>");
            incrIndent();
            for (SqliteCreateTable::Column::Constraint* constr : col->constraints)
            {
                writeln("<constraint>");
                incrIndent();
                writeln("<type>" + constr->typeString() + "</type>");
                writeln("<definition>" + constr->detokenize() + "</definition>");
                decrIndent();
                writeln("</constraint>");
            }
            decrIndent();
            writeln("</constraints>");
        }
        decrIndent();
        writeln("</column>");
    }
    decrIndent();
    writeln("</columns>");

    if (createTable->constraints.size() > 0)
    {
        writeln("<constraints>");
        incrIndent();
        for (SqliteCreateTable::Constraint* constr : createTable->constraints)
        {
            writeln("<constraint>");
            incrIndent();
            writeln("<type>" + constr->typeString() + "</type>");
            writeln("<definition>" + constr->detokenize() + "</definition>");
            decrIndent();
            writeln("</constraint>");
        }
        decrIndent();
        writeln("</constraints>");
    }

    writeln("<rows>");
    incrIndent();
    return true;
}

bool XmlExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(providedData);

    if (isTableExport())
    {
        setupConfig();
        write(docBegin.arg(codecName));
    }

    writeln(QString("<table%1>").arg(isTableExport() ? nsStr : ""));
    incrIndent();

    writeln("<database>" + escape(database) + "</database>");
    writeln("<name>" + escape(table) + "</name>");
    writeln("<virtual>true</name>");
    writeln("<module>" + escape(createTable->module) + "</module>");

    writeln("<ddl>" + escape(ddl) + "</ddl>");

    writeln("<columns>");
    incrIndent();
    for (const QString& col : columnNames)
    {
        writeln("<column>");
        incrIndent();
        writeln("<name>"+ col + "</name>");
        decrIndent();
        writeln("</column>");
    }
    decrIndent();
    writeln("</columns>");

    if (createTable->args.size() > 0)
    {
        writeln("<moduleArgs>");
        incrIndent();
        for (const QString& arg : createTable->args)
            writeln("<arg>" + arg + "</arg>");

        decrIndent();
        writeln("</moduleArgs>");
    }

    writeln("<rows>");
    incrIndent();
    return true;
}

bool XmlExport::exportTableRow(SqlResultsRowPtr data)
{
    return exportQueryResultsRow(data);
}

bool XmlExport::afterExportTable()
{
    decrIndent();
    writeln("</rows>");
    decrIndent();
    writeln("</table>");
    return true;
}

bool XmlExport::beforeExportDatabase(const QString& database)
{
    setupConfig();
    write(docBegin.arg(codecName));

    writeln(QString("<database%1>").arg(nsStr));
    incrIndent();
    writeln("<name>" + escape(database) + "</name>");

    return true;
}

bool XmlExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    writeln("<index>");
    incrIndent();

    writeln("<database>" + escape(database) + "</database>");
    writeln("<name>" + escape(name) + "</name>");
    if (createIndex->uniqueKw)
        writeln("<unique>true</unique>");

    if (createIndex->where)
        writeln("<partial>" + createIndex->where->detokenize() + "</partial>");

    writeln("<ddl>" + escape(ddl) + "</ddl>");

    decrIndent();
    writeln("</index>");
    return true;
}

bool XmlExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    UNUSED(createTrigger);
    writeln("<trigger>");
    incrIndent();

    writeln("<database>" + escape(database) + "</database>");
    writeln("<name>" + escape(name) + "</name>");
    writeln("<ddl>" + escape(ddl) + "</ddl>");

    QString timing = SqliteCreateTrigger::time(createTrigger->eventTime);
    writeln("<timing>" + escape(timing) + "</timing>");

    QString event = createTrigger->event ? SqliteCreateTrigger::Event::typeToString(createTrigger->event->type) : "";
    writeln("<action>" + escape(event) + "</action>");

    QString tag;
    if (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF)
        tag = "<view>";
    else
        tag = "<table>";

    writeln(tag + escape(createTrigger->table) + tag);

    if (createTrigger->precondition)
        writeln("<precondition>" + escape(createTrigger->precondition->detokenize()) + "</precondition>");

    QStringList queryStrings;
    for (SqliteQuery* q : createTrigger->queries)
        queryStrings << q->detokenize();

    writeln("<code>" + escape(queryStrings.join("\n")) + "</code>");

    decrIndent();
    writeln("</trigger>");
    return true;
}

bool XmlExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView)
{
    UNUSED(createView);
    writeln("<view>");
    incrIndent();

    writeln("<database>" + escape(database) + "</database>");
    writeln("<name>" + escape(name) + "</name>");
    writeln("<ddl>" + escape(ddl) + "</ddl>");
    writeln("<select>" + escape(createView->select->detokenize()) + "</select>");
    decrIndent();
    writeln("</view>");
    return true;
}

bool XmlExport::afterExportDatabase()
{
    decrIndent();
    writeln("</database>");
    return true;
}

void XmlExport::setupConfig()
{
    codecName = codec->name();
    indentDepth = 0;
    newLineStr = "";
    indentStr = "";
    indent = (cfg.XmlExport.Format.get() == "format");
    if (indent)
        newLineStr = "\n";

    nsStr = QString();
    if (cfg.XmlExport.UseNamespace.get())
        nsStr = " xmlns=\"" + cfg.XmlExport.Namespace.get() + "\"";

    if (cfg.XmlExport.Escaping.get() == "ampersand")
    {
        useAmpersand = true;
        useCdata = false;
    }
    else if (cfg.XmlExport.Escaping.get() == "cdata")
    {
        useAmpersand = false;
        useCdata = true;
    }
    else
    {
        useAmpersand = true;
        useCdata = true;
    }
}

void XmlExport::incrIndent()
{
    if (indent)
    {
        indentDepth++;
        updateIndent();
    }
}

void XmlExport::decrIndent()
{
    if (indent)
    {
        indentDepth--;
        updateIndent();
    }
}

void XmlExport::updateIndent()
{
    indentStr = QString("    ").repeated(indentDepth);
}

void XmlExport::writeln(const QString& str)
{
    QString newStr;
    if (str.contains("\n"))
    {
        QStringList lines = str.split("\n");
        QMutableStringListIterator it(lines);
        while (it.hasNext())
            it.value().prepend(indentStr);

        newStr = lines.join("\n") + newLineStr;
    }
    else
    {
        newStr = indentStr + str + newLineStr;
    }
    GenericExportPlugin::write(newStr);
}

QString XmlExport::escape(const QString& str)
{
    if (useAmpersand && useCdata)
    {
        if (str.length() >= minLenghtForCdata)
            return escapeCdata(str);
        else
            return escapeAmpersand(str);
    }
    else if (useAmpersand)
    {
        return escapeAmpersand(str);
    }
    else
    {
        return escapeCdata(str);
    }
}

QString XmlExport::escapeCdata(const QString& str)
{
    if (str.contains('"') || str.contains('&') || str.contains('<') || str.contains('>'))
        return "<![CDATA[" + str + "]]>";

    return str;
}

QString XmlExport::escapeAmpersand(const QString& str)
{
    return str.toHtmlEscaped();
}

QString XmlExport::toString(bool value)
{
    return value ? "true" : "false";
}

bool XmlExport::init()
{
    Q_INIT_RESOURCE(xmlexport);
    return GenericExportPlugin::init();
}

void XmlExport::deinit()
{
    Q_CLEANUP_RESOURCE(xmlexport);
}
