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
        SQLITESTUDIO_PLUGIN("csvexport.json")

    public:
        CsvExport();

        QString getFormatName() const override;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const override;
        ExportManager::ExportModes getSupportedModes() const override;
        QString getExportConfigFormName() const override;
        CfgMain* getConfig() override;
        void validateOptions() override;
        QString defaultFileExtension() const override;
        bool beforeExportSingleTable(const QString& database, const QString& table) override;
        bool afterExportSingleTable() override;
        bool beforeExportSingleView(const QString& database, const QString& name) override;
        bool afterExportSingleView() override;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportQueryResultsRow(SqlResultsRowPtr row) override;
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportTableRow(SqlResultsRowPtr data) override;
        bool beforeExportDatabase(const QString& database) override;
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex) override;
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger) override;
        bool exportView(const QString& database, const QString& name, const QStringList& columnNames, const QString& ddl,
                        SqliteCreateViewPtr createView, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportViewRow(SqlResultsRowPtr data) override;
        bool init() override;
        void deinit() override;

    private:
        bool exportTableOrView(const QStringList& columnNames);
        bool exportDataRow(SqlResultsRowPtr data);
        void defineCsvFormat();

        CFG_LOCAL_PERSISTABLE(CsvExportConfig, cfg)
        CsvFormat format;
};

#endif // CSVEXPORT_H
