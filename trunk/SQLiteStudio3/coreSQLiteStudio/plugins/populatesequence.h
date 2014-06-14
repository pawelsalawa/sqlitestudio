#ifndef POPULATESEQUENCE_H
#define POPULATESEQUENCE_H

#include "genericplugin.h"
#include "populateplugin.h"
#include "config_builder.h"

CFG_CATEGORIES(PopulateSequenceConfig,
    CFG_CATEGORY(PopulateSequence,
        CFG_ENTRY(int, StartValue, 0)
        CFG_ENTRY(int, Step,       1)
    )
)

class PopulateSequence : public GenericPlugin, public PopulatePlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN_TITLE("Sequence")
        SQLITESTUDIO_PLUGIN_DESC("Support for populating tables with sequenced values.")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        PopulateSequence();

        QString getTitle() const;
        PopulateEngine* createEngine();
};

class PopulateSequenceEngine : public PopulateEngine
{
    public:
        bool beforePopulating(Db* db, const QString& table);
        QVariant nextValue();
        void afterPopulating();
        CfgMain* getConfig();
        QString getPopulateConfigFormName() const;
        bool validateOptions();

    private:
        CFG_LOCAL(PopulateSequenceConfig, cfg)
        qint64 seq = 0;
        qint64 step = 1;
};

#endif // POPULATESEQUENCE_H
