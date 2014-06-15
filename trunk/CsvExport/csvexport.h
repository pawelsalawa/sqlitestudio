#ifndef CSVEXPORT_H
#define CSVEXPORT_H

#include "csvexport_global.h"
#include "plugins/genericexportplugin.h"
#include "config_builder.h"
#include "csvformat.h"

CFG_CATEGORIES(CsvExportConfig,
     CFG_CATEGORY(CsvExport,
         CFG_ENTRY(bool,    ColumnsInFirstRow, false)
         CFG_ENTRY(int,     Separator,         0)
         CFG_ENTRY(QString, CustomSeparator,   QString())
         CFG_ENTRY(QString, NullValueString,   QString())
     )
)

class CSVEXPORTSHARED_EXPORT CsvExport : public GenericExportPlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN
        SQLITESTUDIO_PLUGIN_TITLE("CSV exporting format")
        SQLITESTUDIO_PLUGIN_DESC("Provides CSV format for exporting")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        CsvExport();

        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportModes getSupportedModes() const;
        QString getExportConfigFormName() const;
        CfgMain* getConfig();
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool afterExportQueryResults();
        bool beforeExportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, bool databaseExport);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExportTable();
        bool beforeExportDatabase();
        bool exportIndex(const QString& database, const QString& name, const QString& ddl);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl);
        bool exportView(const QString& database, const QString& name, const QString& ddl);
        bool afterExportDatabase();

    private:
        void defineCsvFormat();

        CFG_LOCAL(CsvExportConfig, cfg)
        CsvFormat format;
};

#endif // CSVEXPORT_H
