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

        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        QString getExportConfigFormName() const;
        CfgMain* getConfig();
        void validateOptions();
        QString defaultFileExtension() const;
        QString getDefaultEncoding() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool afterExportQueryResults();
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExportTable();
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView);
        bool afterExportDatabase();
        bool beforeExport();
        bool afterExport();
        bool init();
        void deinit();

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

        CFG_LOCAL(JsonExportConfig, cfg)
        QStack<int> elementCounter;
        bool indent = false;
        int indentDepth = 0;
        QString indentStr;
        QString newLineStr;
        QString codecName;
};

#endif // JSONEXPORT_H
