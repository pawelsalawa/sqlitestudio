#ifndef JSONEXPORT_H
#define JSONEXPORT_H

#include "jsonexport_global.h"
#include "plugins/genericexportplugin.h"
#include "config_builder.h"
#include <QStack>

CFG_CATEGORIES(JsonExportConfig,
     CFG_CATEGORY(JsonExport,
         CFG_ENTRY(QString, Format,       "format")
     )
)

class JSONEXPORTSHARED_EXPORT JsonExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("jsonexport.json")

    public:
        JsonExport();

        QString getFormatName() const override;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const override;
        QString getExportConfigFormName() const override;
        CfgMain* getConfig() override;
        void validateOptions() override;
        QString defaultFileExtension() const override;
        QString getDefaultEncoding() const override;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportQueryResultsRow(SqlResultsRowPtr row) override;
        bool afterExportQueryResults() override;
        bool beforeExportSingleTable(const QString& database, const QString& table) override;
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportTableRow(SqlResultsRowPtr data) override;
        bool afterExportTable() override;
        bool afterExportSingleTable() override;
        bool beforeExportDatabase(const QString& database) override;
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex) override;
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger) override;
        bool beforeExportSingleView(const QString& database, const QString& name) override;
        bool exportView(const QString& database, const QString& name, const QStringList& columnNames, const QString& ddl,
                        SqliteCreateViewPtr createView, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportViewRow(SqlResultsRowPtr data) override;
        bool afterExportView() override;
        bool afterExportSingleView() override;
        bool afterExportDatabase() override;
        bool beforeExport() override;
        bool init() override;
        void deinit() override;

    private:
        void setupConfig();
        void incrIndent();
        void decrIndent();
        void updateIndent();
        void incrElementCount();
        void write(const QString& str);
        QString escapeString(const QString& str);
        QString formatValue(const QVariant& val);
        void beginObject();
        void beginObject(const QString& key);
        void endObject();
        void beginArray();
        void beginArray(const QString& key);
        void endArray();
        void writeValue(const QVariant& value);
        void writeValue(const QString& key, const QVariant& value);
        void writePrefixBeforeEnd();
        void writePrefixBeforeNextElement();

        CFG_LOCAL_PERSISTABLE(JsonExportConfig, cfg)
        QStack<int> elementCounter;
        bool indent = false;
        int indentDepth = 0;
        QString indentStr;
        QString newLineStr;
        QString codecName;
};

#endif // JSONEXPORT_H
