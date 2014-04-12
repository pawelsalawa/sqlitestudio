#ifndef SQLEXPORT_H
#define SQLEXPORT_H

#include "plugins/exportplugin.h"
#include "plugins/genericplugin.h"
#include "sqlexport_global.h"
#include "config_builder.h"

CFG_CATEGORIES(SqlExportConfig,
     CFG_CATEGORY(SqlExport,
         CFG_ENTRY(QString, QueryTable, QString::null)
     )
)

CFG_DECLARE(SqlExportConfig)
#define SQL_EXPORT_CFG CFG_INSTANCE(SqlExportConfig)

class SQLEXPORTSHARED_EXPORT SqlExport : public GenericPlugin, public ExportPlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN
        SQLITESTUDIO_PLUGIN_TITLE("SQL exporting format")
        SQLITESTUDIO_PLUGIN_DESC("Provides SQL format for exporting")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportModes getSupportedModes() const;
        CfgMain* getConfig() const;
        QString defaultFileExtension() const;
        QString getConfigFormName(ExportManager::ExportMode mode) const;
        void initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config);
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool afterExportQueryResults();
        bool beforeExportTable(const QString& database, const QString& table, const QString& ddl, bool databaseExport);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExportTable();
        bool beforeExportDatabase();
        bool exportIndex(const QString& database, const QString& name, const QString& ddl);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl);
        bool exportView(const QString& database, const QString& name, const QString& ddl);
        bool afterExportDatabase();
};

#endif // SQLEXPORT_H
