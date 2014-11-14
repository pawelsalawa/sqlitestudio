#include "populatescript.h"
#include "common/unused.h"
#include "services/populatemanager.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"

PopulateScript::PopulateScript()
{
}

QString PopulateScript::getTitle() const
{
    return tr("Script");
}

PopulateEngine* PopulateScript::createEngine()
{
    return new PopulateScriptEngine();
}

bool PopulateScriptEngine::beforePopulating(Db* db, const QString& table)
{
    this->db = db;
    this->table = table;

    evalArgs = {db->getName(), table};

    scriptingPlugin = nullptr;
    for (ScriptingPlugin* plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
    {
        if (plugin->getLanguage() == cfg.PopulateScript.Language.get())
        {
            scriptingPlugin = plugin;
            break;
        }
    }

    if (!scriptingPlugin)
    {
        notifyError(QObject::tr("Could not find plugin to support scripting language: %1").arg(cfg.PopulateScript.Language.get()));
        return false;
    }

    dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(scriptingPlugin);

    context = scriptingPlugin->createContext();

    QString initCode = cfg.PopulateScript.InitCode.get();
    if (!initCode.trimmed().isEmpty())
    {
        if (dbAwarePlugin)
            dbAwarePlugin->evaluate(context, initCode, evalArgs, db);
        else
            scriptingPlugin->evaluate(context, initCode, evalArgs);

        if (scriptingPlugin->hasError(context))
        {
            notifyError(QObject::tr("Error while executing populating initial code: %1").arg(scriptingPlugin->getErrorMessage(context)));
            releaseContext();
            return false;
        }
    }

    rowCnt = 1;
    evalArgs << rowCnt;

    return true;
}

QVariant PopulateScriptEngine::nextValue(bool& nextValueError)
{
    QVariant result;
    if (dbAwarePlugin)
        result = dbAwarePlugin->evaluate(context, cfg.PopulateScript.Code.get(), evalArgs, db);
    else
        result = scriptingPlugin->evaluate(context, cfg.PopulateScript.Code.get(), evalArgs);

    if (scriptingPlugin->hasError(context))
    {
        notifyError(QObject::tr("Error while executing populating code: %1").arg(scriptingPlugin->getErrorMessage(context)));
        releaseContext();
        nextValueError = true;
        return QVariant();
    }

    evalArgs[2] = ++rowCnt;

    return result;
}

void PopulateScriptEngine::afterPopulating()
{
    releaseContext();
}

CfgMain* PopulateScriptEngine::getConfig()
{
    return &cfg;
}

QString PopulateScriptEngine::getPopulateConfigFormName() const
{
    return QStringLiteral("PopulateScriptConfig");
}

bool PopulateScriptEngine::validateOptions()
{
    bool langValid = !cfg.PopulateScript.Language.get().isEmpty();
    bool codeValid = !cfg.PopulateScript.Code.get().trimmed().isEmpty();
    QString lang = cfg.PopulateScript.Language.get();

    POPULATE_MANAGER->handleValidationFromPlugin(langValid, cfg.PopulateScript.Language, QObject::tr("Select implementation language."));
    POPULATE_MANAGER->handleValidationFromPlugin(codeValid, cfg.PopulateScript.Code, QObject::tr("Implementation code cannot be empty."));

    POPULATE_MANAGER->propertySetFromPlugin(cfg.PopulateScript.InitCode, PluginServiceBase::LANG_PROPERTY_NAME, lang);
    POPULATE_MANAGER->propertySetFromPlugin(cfg.PopulateScript.Code, PluginServiceBase::LANG_PROPERTY_NAME, lang);

    return langValid && codeValid;
}

void PopulateScriptEngine::releaseContext()
{
    scriptingPlugin->releaseContext(context);
    context = nullptr;
}
