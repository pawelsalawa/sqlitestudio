#ifndef HTMLEXPORT_H
#define HTMLEXPORT_H

#include "htmlexport_global.h"
#include "plugins/genericexportplugin.h"
#include "config_builder.h"

CFG_CATEGORIES(HtmlExportConfig,
    CFG_CATEGORY(HtmlExport,
        CFG_ENTRY(QString, Format,          "compress")
        CFG_ENTRY(bool,    PrintRowNum,     true)
        CFG_ENTRY(bool,    PrintHeader,     true)
        CFG_ENTRY(bool,    PrintDataTypes,  true)
        CFG_ENTRY(bool,    DontEscapeHtml,  false)
        CFG_ENTRY(int,     ByteLengthLimit, 10000)
    )
)
class HTMLEXPORTSHARED_EXPORT HtmlExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("htmlexport.json")

    public:
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        QString getExportConfigFormName() const;
        CfgMain* getConfig();
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool afterExportQueryResults();
        bool beforeExportSingleTable(const QString& database, const QString& table);
        bool afterExportSingleTable();
        bool beforeExportSingleView(const QString& database, const QString& name);
        bool afterExportSingleView();
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExportTable();
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QStringList& columnNames, const QString& ddl,
                        SqliteCreateViewPtr createView, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportViewRow(SqlResultsRowPtr data);
        bool afterExportView();
        bool afterExport();
        bool init();
        void deinit();

    private:
        bool beginDoc(const QString& title);
        bool exportDataRow(SqlResultsRowPtr data);
        void setupConfig();
        void incrIndent();
        void decrIndent();
        void updateIndent();
        void writeln(const QString& str);
        QString escape(const QString& str);

        static QString compressCss(QString css);

        CFG_LOCAL_PERSISTABLE(HtmlExportConfig, cfg)
        bool indent = false;
        int indentDepth = 0;
        QString indentStr;
        QString newLineStr;
        QString codecName;
        int currentDataRow = 0;
        QList<DataType> columnTypes;
        bool printRownum = false;
        bool printHeader = false;
        bool printDatatypes = false;
        int byteLengthLimit = 0;
};

#endif // HTMLEXPORT_H
