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

        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        CfgMain* getConfig();
        QString defaultFileExtension() const;
        QString getExportConfigFormName() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExport();
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView);
        void validateOptions();
        bool init();
        void deinit();

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

        QString theTable;
        QString columns;
        QStringList tableGeneratedColumns;
        QList<int> generatedColumnIndexes;
        CFG_LOCAL_PERSISTABLE(SqlExportConfig, cfg)
};

#endif // SQLEXPORT_H
