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
        QString getFormatName() const override;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const override;
        QString getExportConfigFormName() const override;
        CfgMain* getConfig() override;
        void validateOptions() override;
        QString defaultFileExtension() const override;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportQueryResultsRow(SqlResultsRowPtr row) override;
        bool afterExportQueryResults() override;
        bool beforeExportSingleTable(const QString& database, const QString& table) override;
        bool afterExportSingleTable() override;
        bool beforeExportSingleView(const QString& database, const QString& name) override;
        bool afterExportSingleView() override;
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportTableRow(SqlResultsRowPtr data) override;
        bool afterExportTable() override;
        bool beforeExportDatabase(const QString& database) override;
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex) override;
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger) override;
        bool exportView(const QString& database, const QString& name, const QStringList& columnNames, const QString& ddl,
                        SqliteCreateViewPtr createView, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) override;
        bool exportViewRow(SqlResultsRowPtr data) override;
        bool afterExportView() override;
        bool afterExport() override;
        bool init() override;
        void deinit() override;

    private:
        bool beginDoc(const QString& title);
        bool exportDataRow(SqlResultsRowPtr data);
        void setupConfig();
        void incrIndent();
        void decrIndent();
        void updateIndent();
        void writeln(const QString& str);
        QString escape(const QString& str);

        static QString compressCssOrJs(QString css);

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
