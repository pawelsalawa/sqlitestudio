#include "csvexport.h"
#include "services/exportmanager.h"
#include "csvserializer.h"

CsvExport::CsvExport()
{
}

QString CsvExport::getFormatName() const
{
    return QStringLiteral("CSV");
}

ExportManager::StandardConfigFlags CsvExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

ExportManager::ExportModes CsvExport::getSupportedModes() const
{
    return ExportManager::FILE|ExportManager::TABLE|ExportManager::QUERY_RESULTS|ExportManager::CLIPBOARD;
}

QString CsvExport::getExportConfigFormName() const
{
    return QStringLiteral("CsvExport");
}

CfgMain* CsvExport::getConfig()
{
    return &cfg;
}

void CsvExport::validateOptions()
{
    if (cfg.CsvExport.Separator.get() >= 4)
    {
        EXPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvExport.CustomSeparator, true, true);

        bool valid = !cfg.CsvExport.CustomSeparator.get().isEmpty();
        EXPORT_MANAGER->handleValidationFromPlugin(valid, cfg.CsvExport.CustomSeparator, tr("Enter the custom separator character."));
    }
    else
    {
        EXPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvExport.CustomSeparator, true, false);
        EXPORT_MANAGER->handleValidationFromPlugin(true, cfg.CsvExport.CustomSeparator);
    }
}

QString CsvExport::defaultFileExtension() const
{
    return QStringLiteral("csv");
}

bool CsvExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    Q_UNUSED(query);
    Q_UNUSED(providedData);
    defineCsvFormat();

    if (cfg.CsvExport.ColumnsInFirstRow.get())
    {
        QStringList cols;
        for (QueryExecutor::ResultColumnPtr resCol : columns)
            cols << resCol->displayName;

        writeln(CsvSerializer::serialize(cols, format));
    }
    return true;
}

bool CsvExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    exportTableRow(row);
    return true;
}

bool CsvExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    Q_UNUSED(database);
    Q_UNUSED(table);
    Q_UNUSED(ddl);
    Q_UNUSED(providedData);
    Q_UNUSED(createTable);
    return exportTable(columnNames);
}

bool CsvExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    Q_UNUSED(database);
    Q_UNUSED(table);
    Q_UNUSED(ddl);
    Q_UNUSED(providedData);
    Q_UNUSED(createTable);
    return exportTable(columnNames);
}

bool CsvExport::exportTable(const QStringList& columnNames)
{
    if (!isTableExport())
        return false;

    defineCsvFormat();
    if (cfg.CsvExport.ColumnsInFirstRow.get())
        writeln(CsvSerializer::serialize(columnNames, format));

    return true;
}

bool CsvExport::exportTableRow(SqlResultsRowPtr data)
{
    QStringList valList;
    QString nl = cfg.CsvExport.NullValueString.get();
    for (const QVariant& val : data->valueList())
        valList << (isNull(val) ? nl : val.toString());

    writeln(CsvSerializer::serialize(valList, format));
    return true;
}

bool CsvExport::beforeExportDatabase(const QString& database)
{
    Q_UNUSED(database);
    return false;
}

bool CsvExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    Q_UNUSED(database);
    Q_UNUSED(name);
    Q_UNUSED(ddl);
    Q_UNUSED(createIndex);
    if (isTableExport())
        return true;

    return false;
}

bool CsvExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    Q_UNUSED(database);
    Q_UNUSED(name);
    Q_UNUSED(ddl);
    Q_UNUSED(createTrigger);
    if (isTableExport())
        return true;

    return false;
}

bool CsvExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView)
{
    Q_UNUSED(database);
    Q_UNUSED(name);
    Q_UNUSED(ddl);
    Q_UNUSED(createView);
    return false;
}

void CsvExport::defineCsvFormat()
{
    format = CsvFormat();
    format.rowSeparator = '\n';

    switch (cfg.CsvExport.Separator.get())
    {
        case 0:
            format.columnSeparator = ',';
            break;
        case 1:
            format.columnSeparator = ';';
            break;
        case 2:
            format.columnSeparator = '\t';
            break;
        case 3:
            format.columnSeparator = ' ';
            break;
        default:
            format.columnSeparator = cfg.CsvExport.CustomSeparator.get();
            break;
    }

    format.calculateSeparatorMaxLengths();
}


bool CsvExport::init()
{
    SQLS_INIT_RESOURCE(csvexport);
    return GenericExportPlugin::init();
}

void CsvExport::deinit()
{
    SQLS_CLEANUP_RESOURCE(csvexport);
}
