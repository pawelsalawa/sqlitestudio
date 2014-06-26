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

        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        QString getExportConfigFormName() const;
        CfgMain* getConfig();
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool afterExportQueryResults();
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable);
        bool exportTableRow(SqlResultsRowPtr data);
        bool afterExportTable();
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr createView);
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

        static QString toString(bool value);

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
