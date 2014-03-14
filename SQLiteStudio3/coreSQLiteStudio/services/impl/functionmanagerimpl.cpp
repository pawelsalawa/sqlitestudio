#include "functionmanagerimpl.h"
#include "plugins/sqlfunctionplugin.h"
#include "services/config.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include <QVariant>
#include <QDebug>

FunctionManagerImpl::FunctionManagerImpl()
{
    init();
}

void FunctionManagerImpl::setFunctions(const QList<FunctionManagerImpl::FunctionPtr>& newFunctions)
{
    functions = newFunctions;
    refreshFunctionsByKey();
    storeInConfig();
    emit functionListChanged();
}

QList<FunctionManager::FunctionPtr> FunctionManagerImpl::getAllFunctions() const
{
    return functions;
}

QList<FunctionManager::FunctionPtr> FunctionManagerImpl::getFunctionsForDatabase(const QString& dbName) const
{
    QList<FunctionPtr> results;
    foreach (const FunctionPtr& func, functions)
    {
        if (func->allDatabases || func->databases.contains(dbName, Qt::CaseInsensitive))
            results << func;
    }
    return results;
}

QVariant FunctionManagerImpl::evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok)
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

void FunctionManagerImpl::evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString,QVariant>& aggregateStorage)
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

void FunctionManagerImpl::evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString,QVariant>& aggregateStorage)
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

QVariant FunctionManagerImpl::evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString,QVariant>& aggregateStorage)
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

void FunctionManagerImpl::init()
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

void FunctionManagerImpl::refreshFunctionsByKey()
{
    functionsByKey.clear();
    foreach (const FunctionPtr& func, functions)
        functionsByKey[Key(func)] = func;
}

void FunctionManagerImpl::storeInConfig()
{
    if (!CFG->setFunctions(functions))
    {
        notifyWarn(tr("Could not store custom SQL functions in configuration file. "
                         "You can try editing functions and save them again. "
                         "Otherwise all modifications will be lost after application restart. "
                         "Error details: %1").arg(CFG->getLastErrorString()));
    }
}

QString FunctionManagerImpl::cannotFindFunctionError(const QString& name, int argCount)
{
    QStringList argMarkers = getArgMarkers(argCount);
    return tr("No such function registered in SQLiteStudio: %1(%2)").arg(name).arg(argMarkers.join(","));
}

QString FunctionManagerImpl::langUnsupportedError(const QString& name, int argCount, const QString& lang)
{
    QStringList argMarkers = getArgMarkers(argCount);
    return tr("Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.")
            .arg(name).arg(argMarkers.join(",")).arg(lang);
}

QStringList FunctionManagerImpl::getArgMarkers(int argCount)
{
    QStringList argMarkers;
    for (int i = 0; i < argCount; i++)
        argMarkers << "?";

    return argMarkers;
}

void FunctionManagerImpl::pluginLoaded(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<SqlFunctionPlugin>())
        return;

    SqlFunctionPlugin* fnPlugin = dynamic_cast<SqlFunctionPlugin*>(plugin);
    if (!fnPlugin)
    {
        qCritical() << "Could not cast to SqlFunctionPlugin, while the type was intended for that plugin! (in FunctionManagerImpl::pluginLoaded())";
        return;
    }

    functionPlugins[fnPlugin->getLanguageName()] = fnPlugin;
}

void FunctionManagerImpl::pluginUnloaded(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<SqlFunctionPlugin>())
        return;

    SqlFunctionPlugin* fnPlugin = dynamic_cast<SqlFunctionPlugin*>(plugin);
    if (!fnPlugin)
    {
        qCritical() << "Could not cast to SqlFunctionPlugin, while the type was intended for that plugin! (in FunctionManagerImpl::pluginUnloaded())";
        return;
    }

    functionPlugins.remove(fnPlugin->getLanguageName());
}

FunctionManagerImpl::Function::Function()
{
}

int qHash(const FunctionManagerImpl::Key& key)
{
    return qHash(key.name) ^ key.argCount ^ static_cast<int>(key.type);
}

bool operator==(const FunctionManagerImpl::Key& key1, const FunctionManagerImpl::Key& key2)
{
    return key1.name == key2.name && key1.type == key2.type && key1.argCount == key2.argCount;
}

FunctionManagerImpl::Key::Key()
{
}

FunctionManagerImpl::Key::Key(FunctionManagerImpl::FunctionPtr function) :
    name(function->name), argCount(function->undefinedArgs ? -1 : function->arguments.size()), type(function->type)
{
}
