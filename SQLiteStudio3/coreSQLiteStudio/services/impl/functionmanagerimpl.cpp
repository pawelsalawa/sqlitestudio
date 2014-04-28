#include "functionmanagerimpl.h"
#include "services/config.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "plugins/scriptingplugin.h"
#include "common/unused.h"
#include "common/utils.h"
#include "services/dbmanager.h"
#include <QVariantList>
#include <QHash>
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
    loadFromConfig();
    createDefaultFunctions();
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
    QVariantList list;
    QHash<QString,QVariant> fnHash;
    foreach (const FunctionPtr& func, functions)
    {
        fnHash["name"] = func->name;
        fnHash["lang"] = func->lang;
        fnHash["code"] = func->code;
        fnHash["initCode"] = func->initCode;
        fnHash["finalCode"] = func->finalCode;
        fnHash["databases"] = common(DBLIST->getDbNames(), func->databases);
        fnHash["arguments"] = func->arguments;
        fnHash["type"] = static_cast<int>(func->type);
        fnHash["undefinedArgs"] = func->undefinedArgs;
        fnHash["allDatabases"] = func->allDatabases;
        list << fnHash;
    }
    CFG_CORE.Internal.Functions.set(list);
}

void FunctionManagerImpl::loadFromConfig()
{
    functions.clear();

    QVariantList list = CFG_CORE.Internal.Functions.get();
    QHash<QString,QVariant> fnHash;
    FunctionPtr func;
    for (const QVariant& var : list)
    {
        fnHash = var.toHash();
        func = FunctionPtr::create();
        func->name = fnHash["name"].toString();
        func->lang = fnHash["lang"].toString();
        func->code = fnHash["code"].toString();
        func->initCode = fnHash["initCode"].toString();
        func->finalCode = fnHash["finalCode"].toString();
        func->databases = fnHash["databases"].toStringList();
        func->arguments = fnHash["arguments"].toStringList();
        func->type = static_cast<Function::Type>(fnHash["type"].toInt());
        func->undefinedArgs = fnHash["undefinedArgs"].toBool();
        func->allDatabases = fnHash["allDatabases"].toBool();
        functions << func;
    }
}

void FunctionManagerImpl::createDefaultFunctions()
{
    if (CFG_CORE.Internal.Functions.isPersisted())
        return;

    FunctionPtr func = FunctionPtr::create();
    func->name = QStringLiteral("regexp");
    func->lang = QStringLiteral("QtScript");
    func->code = QStringLiteral("return arguments[1].toString().match(arguments[0]) != null;");
    func->arguments = {QStringLiteral("pattern"), QStringLiteral("arg")};
    func->type = Function::SCALAR;
    func->undefinedArgs = false;
    func->allDatabases = true;
    functions << func;
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

QString FunctionManager::Function::toString() const
{
    static const QString format = "%1(%2)";
    QString args = undefinedArgs ? "..." : arguments.join(", ");
    return format.arg(name).arg(args);
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
