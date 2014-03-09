#include "functionmanager.h"
#include "notifymanager.h"
#include "config.h"
#include "sqlfunctionplugin.h"
#include "pluginmanager.h"

FunctionManager::FunctionManager()
{
    init();
}

void FunctionManager::setFunctions(const QList<FunctionManager::FunctionPtr>& newFunctions)
{
    functions = newFunctions;
    refreshFunctionsByKey();
    storeInConfig();
    emit functionListChanged();
}

QList<FunctionManager::FunctionPtr> FunctionManager::getAllFunctions() const
{
    return functions;
}

QList<FunctionManager::FunctionPtr> FunctionManager::getFunctionsForDatabase(const QString& dbName) const
{
    QList<FunctionPtr> results;
    foreach (const FunctionPtr& func, functions)
    {
        if (func->allDatabases || func->databases.contains(dbName, Qt::CaseInsensitive))
            results << func;
    }
    return results;
}

QVariant FunctionManager::evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = Function::SCALAR;
    if (!functionsByKey.contains(key))
    {
        ok = false;
        return cannotFindFunctionError(name, argCount);
    }

    FunctionPtr function = functionsByKey[key];
    if (!functionPlugins.contains(function->lang))
    {
        ok = false;
        return langUnsupportedError(name, argCount, function->lang);
    }

    SqlFunctionPlugin* plugin = functionPlugins[function->lang];
    return plugin->evaluateScalar(db, name, function->code, args, ok);
}

void FunctionManager::evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString,QVariant>& aggregateStorage)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = Function::AGGREGATE;
    if (!functionsByKey.contains(key))
        return;

    FunctionPtr function = functionsByKey[key];
    if (!functionPlugins.contains(function->lang))
        return;

    SqlFunctionPlugin* plugin = functionPlugins[function->lang];
    plugin->evaluateAggregateInitial(db, name, argCount, function->initCode, aggregateStorage);
}

void FunctionManager::evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString,QVariant>& aggregateStorage)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = Function::AGGREGATE;
    if (!functionsByKey.contains(key))
        return;

    FunctionPtr function = functionsByKey[key];
    if (!functionPlugins.contains(function->lang))
        return;

    SqlFunctionPlugin* plugin = functionPlugins[function->lang];
    plugin->evaluateAggregateStep(db, name, function->code, args, aggregateStorage);
}

QVariant FunctionManager::evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString,QVariant>& aggregateStorage)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = Function::AGGREGATE;
    if (!functionsByKey.contains(key))
    {
        ok = false;
        return cannotFindFunctionError(name, argCount);
    }

    FunctionPtr function = functionsByKey[key];
    if (!functionPlugins.contains(function->lang))
    {
        ok = false;
        return langUnsupportedError(name, argCount, function->lang);
    }

    SqlFunctionPlugin* plugin = functionPlugins[function->lang];
    return plugin->evaluateAggregateFinal(db, name, argCount, function->finalCode, ok, aggregateStorage);
}

QString FunctionManager::Function::typeString(Type type)
{
    switch (type)
    {
        case Function::SCALAR:
            return "SCALAR";
        case Function::AGGREGATE:
            return "AGGREGATE";
    }
    return QString::null;
}

FunctionManager::Function::Type FunctionManager::Function::typeString(const QString& type)
{
    if (type == "SCALAR")
        return Function::SCALAR;

    if (type == "AGGREGATE")
        return Function::AGGREGATE;

    return Function::SCALAR;
}

void FunctionManager::init()
{
    functions = CFG->getFunctions();
    refreshFunctionsByKey();

    // Read SQL function plugins and connect signals to keep the list up to date
    functionPlugins.clear();
    foreach (SqlFunctionPlugin* plugin, PLUGINS->getLoadedPlugins<SqlFunctionPlugin>())
        functionPlugins[plugin->getLanguageName()] = plugin;

    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginUnloaded(Plugin*,PluginType*)));
}

void FunctionManager::refreshFunctionsByKey()
{
    functionsByKey.clear();
    foreach (const FunctionPtr& func, functions)
        functionsByKey[Key(func)] = func;
}

void FunctionManager::storeInConfig()
{
    if (!CFG->setFunctions(functions))
    {
        notifyWarn(tr("Could not store custom SQL functions in configuration file. "
                         "You can try editing functions and save them again. "
                         "Otherwise all modifications will be lost after application restart. "
                         "Error details: %1").arg(CFG->getLastErrorString()));
    }
}

QString FunctionManager::cannotFindFunctionError(const QString& name, int argCount)
{
    QStringList argMarkers = getArgMarkers(argCount);
    return tr("No such function registered in SQLiteStudio: %1(%2)").arg(name).arg(argMarkers.join(","));
}

QString FunctionManager::langUnsupportedError(const QString& name, int argCount, const QString& lang)
{
    QStringList argMarkers = getArgMarkers(argCount);
    return tr("Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.")
            .arg(name).arg(argMarkers.join(",")).arg(lang);
}

QStringList FunctionManager::getArgMarkers(int argCount)
{
    QStringList argMarkers;
    for (int i = 0; i < argCount; i++)
        argMarkers << "?";

    return argMarkers;
}

void FunctionManager::pluginLoaded(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<SqlFunctionPlugin>())
        return;

    SqlFunctionPlugin* fnPlugin = dynamic_cast<SqlFunctionPlugin*>(plugin);
    if (!fnPlugin)
    {
        qCritical() << "Could not cast to SqlFunctionPlugin, while the type was intended for that plugin! (in FunctionManager::pluginLoaded())";
        return;
    }

    functionPlugins[fnPlugin->getLanguageName()] = fnPlugin;
}

void FunctionManager::pluginUnloaded(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<SqlFunctionPlugin>())
        return;

    SqlFunctionPlugin* fnPlugin = dynamic_cast<SqlFunctionPlugin*>(plugin);
    if (!fnPlugin)
    {
        qCritical() << "Could not cast to SqlFunctionPlugin, while the type was intended for that plugin! (in FunctionManager::pluginUnloaded())";
        return;
    }

    functionPlugins.remove(fnPlugin->getLanguageName());
}

FunctionManager::Function::Function()
{
}

int qHash(const FunctionManager::Key& key)
{
    return qHash(key.name) ^ key.argCount ^ static_cast<int>(key.type);
}

bool operator==(const FunctionManager::Key& key1, const FunctionManager::Key& key2)
{
    return key1.name == key2.name && key1.type == key2.type && key1.argCount == key2.argCount;
}

FunctionManager::Key::Key()
{
}

FunctionManager::Key::Key(FunctionManager::FunctionPtr function) :
    name(function->name), argCount(function->undefinedArgs ? -1 : function->arguments.size()), type(function->type)
{
}
