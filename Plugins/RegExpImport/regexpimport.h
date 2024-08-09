#ifndef REGEXPIMPORT_H
#define REGEXPIMPORT_H

#include "regexpimport_global.h"
#include "plugins/genericplugin.h"
#include "plugins/importplugin.h"
#include "config_builder.h"

class QRegularExpression;
class QFile;
class EncodedTextStream;

CFG_CATEGORIES(RegExpImportConfig,
     CFG_CATEGORY(RegExpImport,
         CFG_ENTRY(QString, Pattern,           QString())
         CFG_ENTRY(QString, GroupsMode,        "all") // all / custom
         CFG_ENTRY(QString, CustomGroupList,   QString())
     )
)

class REGEXPIMPORTSHARED_EXPORT RegExpImport : public GenericPlugin, public ImportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("regexpimport.json")

    public:
        RegExpImport();

        bool init();
        void deinit();
        QString getDataSourceTypeName() const;
        ImportManager::StandardConfigFlags standardOptionsToEnable() const;
        QString getFileFilter() const;
        bool beforeImport(const ImportManager::StandardImportConfig& config);
        void afterImport();
        QList<ColumnDefinition> getColumns() const;
        QList<QVariant> next();
        CfgMain* getConfig();
        QString getImportConfigFormName() const;
        bool validateOptions();

    private:
        CFG_LOCAL_PERSISTABLE(RegExpImportConfig, cfg)
        QRegularExpression* re = nullptr;
        QList<QVariant> groups;
        QStringList columns;
        QFile* file = nullptr;
        EncodedTextStream* stream = nullptr;
        QString buffer;
};

#endif // REGEXPIMPORT_H
