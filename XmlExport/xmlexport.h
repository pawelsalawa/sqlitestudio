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

        SQLITESTUDIO_PLUGIN
        SQLITESTUDIO_PLUGIN_TITLE("XML exporting format")
        SQLITESTUDIO_PLUGIN_DESC("Provides XML format for exporting")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        XmlExport();

        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        QString getExportConfigFormName() const;
        CfgMain* getConfig();
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool afterExportQueryResults();
        bool beforeExportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, bool databaseExport);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExportTable();
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl);
        bool exportView(const QString& database, const QString& name, const QString& ddl);
        bool afterExportDatabase();

    private:
        void setupConfig();
        void incrIndent();
        void decrIndent();
        void updateIndent();
        void writeln(const QString& str);
        QString escape(const QString& str);
        QString escapeCdata(const QString& str);
        QString escapeAmpersand(const QString& str);

        CFG_LOCAL(XmlExportConfig, cfg)
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
