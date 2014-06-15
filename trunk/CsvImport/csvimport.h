#ifndef CSVIMPORT_H
#define CSVIMPORT_H

#include "csvimport_global.h"
#include "plugins/importplugin.h"
#include "plugins/genericplugin.h"
#include "config_builder.h"
#include "csvserializer.h"

CFG_CATEGORIES(CsvImportConfig,
     CFG_CATEGORY(CsvImport,
         CFG_ENTRY(bool,    FirstRowAsColumns, false)
         CFG_ENTRY(int,     Separator,         0)
         CFG_ENTRY(QString, CustomSeparator,   QString())
         CFG_ENTRY(bool,    NullValues,        false)
         CFG_ENTRY(QString, NullValueString,   QString())
     )
)

class QFile;
class QTextStream;

class CSVIMPORTSHARED_EXPORT CsvImport : public GenericPlugin, public ImportPlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN
        SQLITESTUDIO_PLUGIN_TITLE("CSV import")
        SQLITESTUDIO_PLUGIN_DESC("CSV format support for importing data")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        CsvImport();

        QString getDataSourceTypeName() const;
        ImportManager::StandardConfigFlags standardOptionsToEnable() const;
        bool beforeImport(const ImportManager::StandardImportConfig& config);
        void afterImport();
        QList<ColumnDefinition> getColumns() const;
        QList<QVariant> next();
        CfgMain* getConfig();
        QString getImportConfigFormName() const;
        void validateOptions();
        QString getFileFilter() const;

    private:
        bool extractColumns();
        void defineCsvFormat();

        QFile* file = nullptr;
        QTextStream* stream = nullptr;
        QStringList columnNames;
        CsvFormat csvFormat;
        CFG_LOCAL(CsvImportConfig, cfg)
};

#endif // CSVIMPORT_H
