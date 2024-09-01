#include "jsonexport.h"
#include "common/unused.h"
#include <QJsonDocument>

JsonExport::JsonExport()
{
}

QString JsonExport::getFormatName() const
{
    return "JSON";
}

ExportManager::StandardConfigFlags JsonExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

QString JsonExport::getDefaultEncoding() const
{
    return "UTF-8";
}

QString JsonExport::getExportConfigFormName() const
{
    return "JsonExportConfig";
}

CfgMain* JsonExport::getConfig()
{
    return &cfg;
}

void JsonExport::validateOptions()
{
}

QString JsonExport::defaultFileExtension() const
{
    return "json";
}

bool JsonExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(providedData);

    beginObject();
    writeValue("type", "query results");
    writeValue("query", query);

    beginArray("columns");
    QList<DataType> columnTypes = QueryExecutor::resolveColumnTypes(db, columns, true);
    int i = 0;
    for (QueryExecutor::ResultColumnPtr col : columns)
    {
        DataType& type = columnTypes[i];

        beginObject();
        writeValue("displayName", col->displayName);
        writeValue("name", col->column);
        writeValue("database", col->database);
        writeValue("table", col->table);
        writeValue("type", type.toFullTypeString());
        endObject();
    }
    endArray();

    beginArray("rows");
    return true;
}

bool JsonExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    beginArray();
    for (const QVariant& value : row->valueList())
        writeValue(value);

    endArray();
    return true;
}

bool JsonExport::afterExportQueryResults()
{
    endArray();
    endObject();
    return true;
}

bool JsonExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(providedData);
    UNUSED(columnNames);

    beginObject();
    writeValue("type", "table");
    writeValue("database", database);
    writeValue("name", table);
    writeValue("withoutRowId", createTable->withOutRowId);
    writeValue("strict", createTable->strict);
    writeValue("ddl", ddl);

    beginArray("columns");
    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        beginObject();
        writeValue("name", col->name);
        writeValue("type", col->type ? col->type->toDataType().toFullTypeString() : "");
        if (col->constraints.size() > 0)
        {
            beginArray("constraints");
            for (SqliteCreateTable::Column::Constraint* constr : col->constraints)
            {
                beginObject();
                writeValue("type", constr->typeString());
                writeValue("definition", constr->detokenize());
                endObject();
            }
            endArray();
        }
        endObject();
    }
    endArray();

    if (createTable->constraints.size() > 0)
    {
        beginArray("constraints");
        for (SqliteCreateTable::Constraint* constr : createTable->constraints)
        {
            beginObject();
            writeValue("type", constr->typeString());
            writeValue("definition", constr->detokenize());
            endObject();
        }
        endArray();
    }

    beginArray("rows");
    return true;
}

bool JsonExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(providedData);

    beginObject();
    writeValue("type", "table");
    writeValue("database", database);
    writeValue("name", table);
    writeValue("virtual", true);
    writeValue("module", createTable->module);
    writeValue("ddl", ddl);

    beginArray("columns");
    for (const QString& col : columnNames)
        writeValue(col);

    endArray();

    if (createTable->args.size() > 0)
    {
        beginArray("moduleArgs");
        for (const QString& arg : createTable->args)
            writeValue(arg);

        endArray();
    }

    beginArray("rows");
    return true;
}

bool JsonExport::exportTableRow(SqlResultsRowPtr data)
{
    return exportQueryResultsRow(data);
}

bool JsonExport::afterExportTable()
{
    endArray();
    endObject();
    return true;
}

bool JsonExport::beforeExportDatabase(const QString& database)
{
    beginObject();
    writeValue("type", "database");
    writeValue("name", database);
    beginArray("objects");
    return true;
}

bool JsonExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    beginObject();
    writeValue("type", "index");
    writeValue("database", database);
    writeValue("name", name);
    writeValue("unique", createIndex->uniqueKw);

    if (createIndex->where)
        writeValue("partial", createIndex->where->detokenize());

    writeValue("ddl", ddl);
    endObject();
    return true;
}

bool JsonExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    beginObject();
    writeValue("type", "trigger");
    writeValue("database", database);
    writeValue("name", name);
    writeValue("ddl", ddl);

    QString timing = SqliteCreateTrigger::time(createTrigger->eventTime);
    writeValue("timing", timing);

    QString event = createTrigger->event ? SqliteCreateTrigger::Event::typeToString(createTrigger->event->type) : "";
    writeValue("action", event);

    QString obj;
    if (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF)
        obj = "view";
    else
        obj = "table";

    writeValue(obj, createTrigger->table);

    if (createTrigger->precondition)
        writeValue("precondition", createTrigger->precondition->detokenize());

    QStringList queryStrings;
    for (SqliteQuery* q : createTrigger->queries)
        queryStrings << q->detokenize();

    writeValue("code", queryStrings.join("\n"));
    endObject();
    return true;
}

bool JsonExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView)
{
    beginObject();
    writeValue("type", "view");
    writeValue("database", database);
    writeValue("name", name);
    writeValue("ddl", ddl);
    writeValue("select", createView->select->detokenize());
    endObject();
    return true;
}

bool JsonExport::afterExportDatabase()
{
    endArray();
    endObject();
    return true;
}

bool JsonExport::beforeExport()
{
    setupConfig();
    return true;
}

bool JsonExport::init()
{
    SQLS_INIT_RESOURCE(jsonexport);
    return GenericExportPlugin::init();
}

void JsonExport::deinit()
{
    SQLS_CLEANUP_RESOURCE(jsonexport);
}

void JsonExport::setupConfig()
{
    elementCounter.clear();
    elementCounter.push(0);
    indent = (cfg.JsonExport.Format.get() == "format");
    indentDepth = 0;
    updateIndent();
}

void JsonExport::incrIndent()
{
    elementCounter.push(0);
    if (!indent)
        return;

    indentDepth++;
    updateIndent();
}

void JsonExport::decrIndent()
{
    elementCounter.pop();
    if (!indent)
        return;

    indentDepth--;
    updateIndent();
}

void JsonExport::updateIndent()
{
    static const QString singleIndent = QStringLiteral("    ");
    indentStr = singleIndent.repeated(indentDepth);
}

void JsonExport::incrElementCount()
{
    elementCounter.top()++;
}

void JsonExport::write(const QString& str)
{
    GenericExportPlugin::write(indentStr + str);
}

QString JsonExport::escapeString(const QString& str)
{
    QString copy = str;
    return "\"" +
            copy.replace("\\", "\\\\")
                .replace("\"", "\\\"")
                .replace("/", "\\/")
                .replace("\b", "\\b")
                .replace("\f", "\\f")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t")
            + "\"";
}

QString JsonExport::formatValue(const QVariant& val)
{
    if (val.isNull())
        return "null";

    switch (val.userType())
    {
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
        case QMetaType::Double:
        case QMetaType::Bool:
            return val.toString();
        default:
            break;
    }

    return escapeString(val.toString());
}

void JsonExport::beginObject()
{
    static const QString formatted = QStringLiteral("{\n");
    static const QString compact = QStringLiteral("{");

    writePrefixBeforeNextElement();
    write(indent ? formatted : compact);
    incrIndent();
}

void JsonExport::beginObject(const QString& key)
{
    static const QString formatted = QStringLiteral("%1: {\n");
    static const QString compact = QStringLiteral("%1:{");

    QString escaped = escapeString(key);

    writePrefixBeforeNextElement();
    write(indent ? formatted.arg(escaped) : compact.arg(escaped));
    incrIndent();
}

void JsonExport::endObject()
{
    writePrefixBeforeEnd();
    decrIndent();
    write("}");
    incrElementCount();
}

void JsonExport::beginArray()
{
    static const QString formatted = QStringLiteral("[\n");
    static const QString compact = QStringLiteral("[");

    writePrefixBeforeNextElement();
    write(indent ? formatted : compact);
    incrIndent();
}

void JsonExport::beginArray(const QString& key)
{
    static const QString formatted = QStringLiteral("%1: [\n");
    static const QString compact = QStringLiteral("%1:[");

    QString escaped = escapeString(key);

    writePrefixBeforeNextElement();
    write(indent ? formatted.arg(escaped) : compact.arg(escaped));
    incrIndent();
}

void JsonExport::endArray()
{
    writePrefixBeforeEnd();
    decrIndent();
    write("]");
    incrElementCount();
}

void JsonExport::writeValue(const QVariant& value)
{
    writePrefixBeforeNextElement();
    write(formatValue(value));
    incrElementCount();
}

void JsonExport::writeValue(const QString& key, const QVariant& value)
{
    static const QString formatted = QStringLiteral("%1: %2");
    static const QString compact = QStringLiteral("%1:%2");

    QString escaped = escapeString(key);
    QString val = formatValue(value);

    writePrefixBeforeNextElement();
    write(indent ? formatted.arg(escaped, val) : compact.arg(escaped, val));
    incrElementCount();
}

void JsonExport::writePrefixBeforeEnd()
{
    if (indent && elementCounter.top() > 0)
        GenericExportPlugin::write("\n");
}

void JsonExport::writePrefixBeforeNextElement()
{
    if (elementCounter.top() > 0)
    {
        GenericExportPlugin::write(",");
        if (indent)
            GenericExportPlugin::write("\n");
    }
}
