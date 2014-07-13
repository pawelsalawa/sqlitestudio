#include "sqlexport.h"
#include "common/utils_sql.h"
#include "sqlitestudio.h"
#include "config_builder.h"
#include "services/exportmanager.h"
#include "common/unused.h"
#include <QTextCodec>

SqlExport::SqlExport()
{
}

QString SqlExport::getFormatName() const
{
    return "SQL";
}

ExportManager::StandardConfigFlags SqlExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

CfgMain* SqlExport::getConfig()
{
    return &cfg;
}

QString SqlExport::defaultFileExtension() const
{
    return ".sql";
}

QString SqlExport::getExportConfigFormName() const
{
    if (exportMode == ExportManager::QUERY_RESULTS)
        return "sqlExportQueryConfig";

    return QString::null;
}

bool SqlExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(providedData);
    Dialect dialect = db->getDialect();
    QStringList colDefs;
    for (QueryExecutor::ResultColumnPtr resCol : columns)
        colDefs << wrapObjIfNeeded(resCol->displayName, dialect);

    this->columns = colDefs.join(", ");

    writeHeader();
    if (cfg.SqlExport.IncludeQueryInComments.get())
    {
        writeln(tr("-- Results of query:"));
        writeln(commentAllSqlLines(query));
        writeln("--");
    }

    writeBegin();

    if (!cfg.SqlExport.GenerateCreateTable.get())
        return true;

    theTable = wrapObjIfNeeded(cfg.SqlExport.QueryTable.get(), dialect);
    QString ddl = "CREATE TABLE " + theTable + " (" + this->columns + ");";
    writeln("");
    writeln(ddl);
    return true;
}

bool SqlExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    QStringList argList = rowToArgList(row);
    QString argStr = argList.join(", ");
    QString sql = "INSERT INTO " + theTable + " (" + this->columns + ") VALUES (" + argStr + ");";
    writeln(sql);
    return true;
}

bool SqlExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(createTable);
    UNUSED(providedData);
    return exportTable(database, table, columnNames, ddl);
}

bool SqlExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(createTable);
    UNUSED(providedData);
    return exportTable(database, table, columnNames, ddl);
}

bool SqlExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl)
{
    Dialect dialect = db->getDialect();

    QStringList colList;
    for (const QString& colName : columnNames)
        colList << wrapObjIfNeeded(colName, dialect);

    columns = colList.join(", ");

    if (isTableExport())
    {
        writeHeader();
        writeFkDisable();
        writeBegin();
    }

    QString fullName = getNameForObject(database, table, false);
    writeln("");
    writeln(tr("-- Table: %1").arg(fullName));

    theTable = getNameForObject(database, table, true, dialect);
    writeln(ddl);
    return true;
}

bool SqlExport::exportTableRow(SqlResultsRowPtr data)
{
    QStringList argList = rowToArgList(data);
    QString argStr = argList.join(", ");
    QString sql = "INSERT INTO " + theTable + " (" + columns + ") VALUES (" + argStr + ");";
    writeln(sql);
    return true;
}

bool SqlExport::afterExport()
{
    writeCommit();
    return true;
}

bool SqlExport::beforeExportDatabase(const QString& database)
{
    UNUSED(database);
    writeHeader();
    writeFkDisable();
    writeBegin();
    return true;
}

bool SqlExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    UNUSED(createIndex);
    QString index = getNameForObject(database, name, false);
    writeln("");
    writeln(tr("-- Index: %1").arg(index));
    writeln(ddl);
    return true;
}

bool SqlExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    UNUSED(createTrigger);
    QString trig = getNameForObject(database, name, false);
    writeln("");
    writeln(tr("-- Trigger: %1").arg(trig));
    writeln(ddl);
    return true;
}

bool SqlExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView)
{
    UNUSED(createView);
    QString view = getNameForObject(database, name, false);
    writeln("");
    writeln(tr("-- View: %1").arg(view));
    writeln(ddl);
    return true;
}

void SqlExport::writeHeader()
{
    QDateTime ctime = QDateTime::currentDateTime();
    writeln("--");
    writeln(tr("-- File generated with SQLiteStudio v%1 on %2").arg(SQLITESTUDIO->getVersionString()).arg(ctime.toString()));
    writeln("--");
    if (standardOptionsToEnable().testFlag(ExportManager::CODEC))
    {
        writeln(tr("-- Text encoding used: %1").arg(QString::fromLatin1(codec->name())));
        writeln("--");
    }
}

void SqlExport::writeBegin()
{
    writeln("BEGIN TRANSACTION;");
}

void SqlExport::writeCommit()
{
    writeln("");
    writeln("COMMIT TRANSACTION;");
}

void SqlExport::writeFkDisable()
{
    writeln("PRAGMA foreign_keys = off;");
}

QString SqlExport::getNameForObject(const QString& database, const QString& name, bool wrapped, Dialect dialect)
{
    QString obj = wrapped ? wrapObjIfNeeded(name, dialect) : name;
    if (!database.isNull() && database.toLower() != "main")
        obj = (wrapped ?  wrapObjIfNeeded(database, dialect) : database) + "." + obj;

    return obj;
}

QStringList SqlExport::rowToArgList(SqlResultsRowPtr row)
{
    QStringList argList;
    for (const QVariant& value : row->valueList())
    {
        switch (value.userType())
        {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                argList << value.toString();
                break;
            case QVariant::Double:
                argList << QString::number(value.toDouble());
                break;
            case QVariant::Bool:
                argList << QString::number(value.toInt());
                break;
            case QVariant::ByteArray:
            {
                if (db->getVersion() >= 3) // version 2 will go to the regular string processing
                {
                    argList << "X'" + value.toByteArray().toHex().toUpper() + "'";
                    break;
                }
            }
            default:
                argList << wrapString(escapeString(value.toString()));
                break;
        }
    }
    return argList;
}

void SqlExport::validateOptions()
{
    if (exportMode == ExportManager::QUERY_RESULTS)
    {
        bool valid = !cfg.SqlExport.QueryTable.get().isEmpty();
        EXPORT_MANAGER->handleValidationFromPlugin(valid, cfg.SqlExport.QueryTable, tr("Table name for INSERT statements is mandatory."));
    }
}

bool SqlExport::init()
{
    Q_INIT_RESOURCE(sqlexport);
    return GenericExportPlugin::init();
}

void SqlExport::deinit()
{
    Q_CLEANUP_RESOURCE(sqlexport);
}
