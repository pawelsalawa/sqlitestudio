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
         CFG_ENTRY(bool,    QuotationMark,     true)
     )
)

class QFile;
class QTextStream;

class CSVIMPORTSHARED_EXPORT CsvImport : public GenericPlugin, public ImportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("csvimport.json")

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
        bool validateOptions();
        QString getFileFilter() const;
        bool init();
        void deinit();

    private:
        bool extractColumns();
        void defineCsvFormat();

        QFile* file = nullptr;
        QTextStream* stream = nullptr;
        QStringList columnNames;
        CsvFormat csvFormat;
        CFG_LOCAL_PERSISTABLE(CsvImportConfig, cfg)
};

#endif // CSVIMPORT_H
