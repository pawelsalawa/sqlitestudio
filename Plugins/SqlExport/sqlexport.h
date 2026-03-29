#ifndef SQLEXPORT_H
#define SQLEXPORT_H

#include "plugins/genericexportplugin.h"
#include "sqlexport_global.h"
#include "config_builder.h"

CFG_CATEGORIES(SqlExportConfig,
     CFG_CATEGORY(SqlExport,
         CFG_ENTRY(QString, QueryTable,             QString())
         CFG_ENTRY(bool,    GenerateCreateTable,    false)
         CFG_ENTRY(bool,    IncludeQueryInComments, true)
         CFG_ENTRY(bool,    UseFormatter,           false)
         CFG_ENTRY(bool,    FormatDdlsOnly,         false)
         CFG_ENTRY(bool,    GenerateIfNotExists,    true)
         CFG_ENTRY(bool,    GenerateDrop,           false)
     )
)

class SQLEXPORTSHARED_EXPORT SqlExport : public GenericExportPlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN("sqlexport.json")

    public:
        SqlExport();

        QString getFormatName() const override;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const override;
        CfgMain* getConfig() override;
        QString defaultFileExtension() const override;
        QString getExportConfigFormName() const override;
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
        bool afterExport() override;
        bool beforeExportDatabase(const QString& database) override;
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex) override;
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger) override;
        bool exportView(const QString& database, const QString& name, const QStringList& columnNames, const QString& ddl,
                        SqliteCreateViewPtr createView, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportViewRow(SqlResultsRowPtr data) override;
        void validateOptions() override;
        bool init() override;
        void deinit() override;

    private:
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl);
        void writeHeader();
        void writeBegin();
        void writeCommit();
        void writeFkDisable();
        void writeFkEnable();
        QString formatQuery(const QString& sql);
        QString getNameForObject(const QString& database, const QString& name, bool wrapped);
        QStringList rowToArgList(SqlResultsRowPtr row, bool honorGeneratedColumns = false);
        bool exportDataRow(SqlResultsRowPtr data);

        QString theTable;
        QString columns;
        QStringList tableGeneratedColumns;
        QList<int> generatedColumnIndexes;
        CFG_LOCAL_PERSISTABLE(SqlExportConfig, cfg)
};

#endif // SQLEXPORT_H
