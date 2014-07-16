#include "jsonexport.h"
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
    return QString();
}

CfgMain* JsonExport::getConfig()
{
    return nullptr;
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
    return true;
}

bool JsonExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    return true;
}

bool JsonExport::afterExportQueryResults()
{
    return true;
}

bool JsonExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    beginObject();
    writeValue("type", "table");
    writeValue("database", database);
    writeValue("name", table);
    writeValue("withoutRowId", createTable->withOutRowId.isNull());
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
    return true;
}

bool JsonExport::exportTableRow(SqlResultsRowPtr data)
{
    return true;
}

bool JsonExport::afterExportTable()
{
    endArray();
    endObject();
    return true;
}

bool JsonExport::beforeExportDatabase(const QString& database)
{
    return true;
}

bool JsonExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    return true;
}

bool JsonExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    return true;
}

bool JsonExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView)
{
    return true;
}

bool JsonExport::afterExportDatabase()
{
    return true;
}

bool JsonExport::beforeExport()
{
    setupConfig();
    elementCounter.clear();
    elementCounter.push(0);
    return true;
}

bool JsonExport::afterExport()
{
    return true;
}

bool JsonExport::init()
{
    return true;
}

void JsonExport::deinit()
{

}

void JsonExport::setupConfig()
{
    indent = false;
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
            copy.replace("\"", "\\\"")
                .replace("\\", "\\\\")
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

    switch (val.type())
    {
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Double:
        case QVariant::Bool:
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
    static const QString formatted = QStringLiteral("%1\n");

    QString val = formatValue(value);

    writePrefixBeforeNextElement();
    write(indent ? formatted.arg(val) : val);
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
