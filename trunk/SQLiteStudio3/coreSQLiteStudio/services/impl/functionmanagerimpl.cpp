#include "functionmanagerimpl.h"
#include "services/config.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "plugins/scriptingplugin.h"
#include "common/unused.h"
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
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(function->lang);
    if (!plugin)
    {
        ok = false;
        return langUnsupportedError(name, argCount, function->lang);
    }
    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    QString error;
    QVariant result;

    if (dbAwarePlugin)
        result = dbAwarePlugin->evaluate(function->code, args, db, false, &error);
    else
        result = plugin->evaluate(function->code, args, &error);

    if (!error.isEmpty())
    {
        ok = false;
        return error;
    }
    return result;
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
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(function->lang);
    if (!plugin)
        return;

    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    ScriptingPlugin::Context* ctx = plugin->createContext();
    aggregateStorage["context"] = QVariant::fromValue(ctx);

    if (dbAwarePlugin)
        dbAwarePlugin->evaluate(ctx, function->code, {}, db, false);
    else
        plugin->evaluate(ctx, function->code, {});

    if (plugin->hasError(ctx))
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorMessage"] = plugin->getErrorMessage(ctx);
    }
}

void FunctionManagerImpl::evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString,QVariant>& aggregateStorage)
{
    UNUSED(db);

    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = Function::AGGREGATE;
    if (!functionsByKey.contains(key))
        return;

    FunctionPtr function = functionsByKey[key];
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(function->lang);
    if (!plugin)
        return;

    if (aggregateStorage.contains("error"))
        return;

    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    ScriptingPlugin::Context* ctx = aggregateStorage["context"].value<ScriptingPlugin::Context*>();
    if (dbAwarePlugin)
        dbAwarePlugin->evaluate(ctx, function->code, args, db, false);
    else
        plugin->evaluate(ctx, function->code, args);

    if (plugin->hasError(ctx))
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorMessage"] = plugin->getErrorMessage(ctx);
    }

}

QVariant FunctionManagerImpl::evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString,QVariant>& aggregateStorage)
{
    UNUSED(db);

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
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(function->lang);
    if (!plugin)
    {
        ok = false;
        return langUnsupportedError(name, argCount, function->lang);
    }

    ScriptingPlugin::Context* ctx = aggregateStorage["context"].value<ScriptingPlugin::Context*>();
    if (aggregateStorage.contains("error"))
    {
        ok = false;
        plugin->releaseContext(ctx);
        return aggregateStorage["errorMessage"];
    }

    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    QVariant result;
    if (dbAwarePlugin)
        result = dbAwarePlugin->evaluate(ctx, function->code, {}, db, false);
    else
        result = plugin->evaluate(ctx, function->code, {});

    if (plugin->hasError(ctx))
    {
        ok = false;
        QString msg = plugin->getErrorMessage(ctx);
        plugin->releaseContext(ctx);
        return msg;
    }

    plugin->releaseContext(ctx);
    return result;
}

void FunctionManagerImpl::init()
{
    functions = CFG->getFunctions();
    refreshFunctionsByKey();
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
