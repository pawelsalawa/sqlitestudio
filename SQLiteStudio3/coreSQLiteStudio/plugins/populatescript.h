#ifndef POPULATESCRIPT_H
#define POPULATESCRIPT_H

#include "builtinplugin.h"
#include "populateplugin.h"
#include "config_builder.h"
#include "scriptingplugin.h"

CFG_CATEGORIES(PopulateScriptConfig,
    CFG_CATEGORY(PopulateScript,
        CFG_ENTRY(QString, Language, QString())
        CFG_ENTRY(QString, InitCode, QString())
        CFG_ENTRY(QString, Code,     QString())
    )
)

/**
 * @brief Populate from evaluated script code
 *
 * Initial code evaluation gets 2 arguments - the db name and the table name.
 * Each evaluation of per-step code gets 3 arguments, 2 of them just like above
 * and the 3rd is number of the row being currently populated (starting from 1).
 */
class PopulateScript : public BuiltInPlugin, public PopulatePlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN_TITLE("Constant")
        SQLITESTUDIO_PLUGIN_DESC("Support for populating tables with a constant value.")
        SQLITESTUDIO_PLUGIN_VERSION(10001)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        PopulateScript();

        QString getTitle() const;
        PopulateEngine* createEngine();
};

class PopulateScriptEngine : public PopulateEngine
{
    public:
        bool beforePopulating(Db* db, const QString& table);
        QVariant nextValue(bool& nextValueError);
        void afterPopulating();
        CfgMain* getConfig();
        QString getPopulateConfigFormName() const;
        bool validateOptions();

    private:
        CFG_LOCAL(PopulateScriptConfig, cfg)
        void releaseContext();

        ScriptingPlugin* scriptingPlugin = nullptr;
        DbAwareScriptingPlugin* dbAwarePlugin = nullptr;
        ScriptingPlugin::Context* context = nullptr;
        Db* db = nullptr;
        QString table;
        int rowCnt = 0;
        QList<QVariant> evalArgs;
};

#endif // POPULATESCRIPT_H
