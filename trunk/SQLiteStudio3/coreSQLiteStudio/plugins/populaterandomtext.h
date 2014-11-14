#ifndef POPULATERANDOMTEXT_H
#define POPULATERANDOMTEXT_H

#include "builtinplugin.h"
#include "populateplugin.h"
#include "config_builder.h"

CFG_CATEGORIES(PopulateRandomTextConfig,
    CFG_CATEGORY(PopulateRandomText,
        CFG_ENTRY(int,     MinLength,    4)
        CFG_ENTRY(int,     MaxLength,    20)
        CFG_ENTRY(bool,    IncludeAlpha,      true)
        CFG_ENTRY(bool,    IncludeNumeric,    true)
        CFG_ENTRY(bool,    IncludeWhitespace, true)
        CFG_ENTRY(bool,    IncludeBinary,     false)
        CFG_ENTRY(bool,    UseCustomSets,     false)
        CFG_ENTRY(QString, CustomCharacters,  QString())
    )
)
class PopulateRandomText : public BuiltInPlugin, public PopulatePlugin
{
       Q_OBJECT

       SQLITESTUDIO_PLUGIN_TITLE("Random text")
       SQLITESTUDIO_PLUGIN_DESC("Support for populating tables with random characters.")
       SQLITESTUDIO_PLUGIN_VERSION(10001)
       SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        PopulateRandomText();

        QString getTitle() const;
        PopulateEngine* createEngine();
};

class PopulateRandomTextEngine : public PopulateEngine
{
    public:
        bool beforePopulating(Db* db, const QString& table);
        QVariant nextValue(bool& nextValueError);
        void afterPopulating();
        CfgMain* getConfig();
        QString getPopulateConfigFormName() const;
        bool validateOptions();

    private:
        CFG_LOCAL(PopulateRandomTextConfig, cfg)
        int range;
        QString chars;
};

#endif // POPULATERANDOMTEXT_H
