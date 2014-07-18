#ifndef DBFEXPORT_H
#define DBFEXPORT_H

#include "qdbf/qdbftable.h"
#include "dbfexport_global.h"
#include "plugins/genericexportplugin.h"

class DBFEXPORTSHARED_EXPORT DbfExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("dbfexport.json")

    public:
        DbfExport();
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportModes getSupportedModes() const;
        QString getExportConfigFormName() const;
        bool isBinaryData() const;
        CfgMain* getConfig();
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportTableRow(SqlResultsRowPtr data);
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView);
        bool init();
        void deinit();

    private:
        bool exportTable(const QStringList& columnNames);

//        CFG_LOCAL(CsvExportConfig, cfg)
        QDbf::QDbfTable* dbfTable = nullptr;
};

#endif // DBFEXPORT_H
