#ifndef XMLEXPORT_H
#define XMLEXPORT_H

#include "xmlexport_global.h"
#include "plugins/genericexportplugin.h"
#include "config_builder.h"

CFG_CATEGORIES(XmlExportConfig,
     CFG_CATEGORY(XmlExport,
         CFG_ENTRY(QString, Format,       "format")
         CFG_ENTRY(bool,    UseNamespace, false)
         CFG_ENTRY(QString, Namespace,    QString())
         CFG_ENTRY(QString, Escaping,     "mixed")
     )
)

class XMLEXPORTSHARED_EXPORT XmlExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("xmlexport.json")

    public:
        XmlExport();

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
        bool beforeExportSingleView(const QString& database, const QString& name) override;
        bool afterExportSingleView() override;
        bool afterExportView() override;
        bool afterExportDatabase() override;
        bool init() override;
        void deinit() override;

    private:
        void setupConfig();
        void incrIndent();
        void decrIndent();
        void updateIndent();
        void writeln(const QString& str);
        QString escape(const QString& str);
        QString escapeCdata(const QString& str);
        QString escapeAmpersand(const QString& str);
        QString tagWithValue(const QString& tag, const QString& value);
        void writeTagWithValue(const QString& tag, const QString& value);

        static QString toString(bool value);

        CFG_LOCAL_PERSISTABLE(XmlExportConfig, cfg)
        bool indent = false;
        int indentDepth = 0;
        QString indentStr;
        QString newLineStr;
        QString nsStr;
        QString codecName;
        bool useAmpersand = true;
        bool useCdata = true;
        static const QString docBegin;

        static constexpr int minLenghtForCdata = 100;
};

#endif // XMLEXPORT_H
