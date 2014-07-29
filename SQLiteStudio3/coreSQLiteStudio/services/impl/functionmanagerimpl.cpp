#include "functionmanagerimpl.h"
#include "services/config.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "plugins/scriptingplugin.h"
#include "common/unused.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "services/dbmanager.h"
#include "db/queryexecutor.h"
#include "db/sqlquery.h"
#include <QVariantList>
#include <QHash>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QUrl>

FunctionManagerImpl::FunctionManagerImpl()
{
    init();
}

void FunctionManagerImpl::setScriptFunctions(const QList<ScriptFunction*>& newFunctions)
{
    clearFunctions();
    functions = newFunctions;
    refreshFunctionsByKey();
    storeInConfig();
    emit functionListChanged();
}

QList<FunctionManager::ScriptFunction*> FunctionManagerImpl::getAllScriptFunctions() const
{
    return functions;
}

QList<FunctionManager::ScriptFunction*> FunctionManagerImpl::getScriptFunctionsForDatabase(const QString& dbName) const
{
    QList<ScriptFunction*> results;
    foreach (ScriptFunction* func, functions)
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
    key.type = ScriptFunction::SCALAR;
    if (functionsByKey.contains(key))
    {
        ScriptFunction* function = functionsByKey[key];
        return evaluateScriptScalar(function, name, argCount, args, db, ok);
    }
    else if (nativeFunctionsByKey.contains(key))
    {
        NativeFunction* function = nativeFunctionsByKey[key];
        return evaluateNativeScalar(function, args, db, ok);
    }

    ok = false;
    return cannotFindFunctionError(name, argCount);
}

void FunctionManagerImpl::evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString,QVariant>& aggregateStorage)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = ScriptFunction::AGGREGATE;
    if (functionsByKey.contains(key))
    {
        ScriptFunction* function = functionsByKey[key];
        evaluateScriptAggregateInitial(function, db, aggregateStorage);
    }
}

void FunctionManagerImpl::evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString,QVariant>& aggregateStorage)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = ScriptFunction::AGGREGATE;
    if (functionsByKey.contains(key))
    {
        ScriptFunction* function = functionsByKey[key];
        evaluateScriptAggregateStep(function, args, db, aggregateStorage);
    }
}

QVariant FunctionManagerImpl::evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString,QVariant>& aggregateStorage)
{
    Key key;
    key.name = name;
    key.argCount = argCount;
    key.type = ScriptFunction::AGGREGATE;
    if (functionsByKey.contains(key))
    {
        ScriptFunction* function = functionsByKey[key];
        return evaluateScriptAggregateFinal(function, name, argCount, db, ok, aggregateStorage);
    }

    ok = false;
    return cannotFindFunctionError(name, argCount);
}

QVariant FunctionManagerImpl::evaluateScriptScalar(ScriptFunction* func, const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok)
{
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(func->lang);
    if (!plugin)
    {
        ok = false;
        return langUnsupportedError(name, argCount, func->lang);
    }
    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    QString error;
    QVariant result;

    if (dbAwarePlugin)
        result = dbAwarePlugin->evaluate(func->code, args, db, false, &error);
    else
        result = plugin->evaluate(func->code, args, &error);

    if (!error.isEmpty())
    {
        ok = false;
        return error;
    }
    return result;
}

void FunctionManagerImpl::evaluateScriptAggregateInitial(ScriptFunction* func, Db* db, QHash<QString, QVariant>& aggregateStorage)
{
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(func->lang);
    if (!plugin)
        return;

    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    ScriptingPlugin::Context* ctx = plugin->createContext();
    aggregateStorage["context"] = QVariant::fromValue(ctx);

    if (dbAwarePlugin)
        dbAwarePlugin->evaluate(ctx, func->code, {}, db, false);
    else
        plugin->evaluate(ctx, func->code, {});

    if (plugin->hasError(ctx))
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorMessage"] = plugin->getErrorMessage(ctx);
    }
}

void FunctionManagerImpl::evaluateScriptAggregateStep(ScriptFunction* func, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage)
{
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(func->lang);
    if (!plugin)
        return;

    if (aggregateStorage.contains("error"))
        return;

    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    ScriptingPlugin::Context* ctx = aggregateStorage["context"].value<ScriptingPlugin::Context*>();
    if (dbAwarePlugin)
        dbAwarePlugin->evaluate(ctx, func->code, args, db, false);
    else
        plugin->evaluate(ctx, func->code, args);

    if (plugin->hasError(ctx))
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorMessage"] = plugin->getErrorMessage(ctx);
    }
}

QVariant FunctionManagerImpl::evaluateScriptAggregateFinal(ScriptFunction* func, const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage)
{
    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(func->lang);
    if (!plugin)
    {
        ok = false;
        return langUnsupportedError(name, argCount, func->lang);
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
        result = dbAwarePlugin->evaluate(ctx, func->code, {}, db, false);
    else
        result = plugin->evaluate(ctx, func->code, {});

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

QList<FunctionManager::NativeFunction*> FunctionManagerImpl::getAllNativeFunctions() const
{
    return nativeFunctions;
}

QVariant FunctionManagerImpl::evaluateNativeScalar(NativeFunction* func, const QList<QVariant>& args, Db* db, bool& ok)
{
    if (!func->undefinedArgs && args.size() != func->arguments.size())
    {
        ok = false;
        return tr("Invalid number of arguments to function '%1'. Expected %2, but got %3.").arg(func->name, QString::number(func->arguments.size()),
                                                                                                QString::number(args.size()));
    }

    return func->functionPtr(args, db, ok);
}

void FunctionManagerImpl::init()
{
    loadFromConfig();
    initNativeFunctions();
    refreshFunctionsByKey();
}

void FunctionManagerImpl::initNativeFunctions()
{
    registerNativeFunction("regexp", {"pattern", "arg"}, FunctionManagerImpl::nativeRegExp);
    registerNativeFunction("sqlfile", {"file"}, FunctionManagerImpl::nativeSqlFile);
    registerNativeFunction("readfile", {"file"}, FunctionManagerImpl::nativeReadFile);
    registerNativeFunction("writefile", {"file", "data"}, FunctionManagerImpl::nativeWriteFile);
    registerNativeFunction("langs", {}, FunctionManagerImpl::nativeLangs);
    registerNativeFunction("script", {"language", "code"}, FunctionManagerImpl::nativeScript);
    registerNativeFunction("html_escape", {"string"}, FunctionManagerImpl::nativeHtmlEscape);
    registerNativeFunction("url_encode", {"string"}, FunctionManagerImpl::nativeUrlEncode);
    registerNativeFunction("url_decode", {"string"}, FunctionManagerImpl::nativeUrlDecode);
    registerNativeFunction("base64_encode", {"data"}, FunctionManagerImpl::nativeBase64Encode);
    registerNativeFunction("base64_decode", {"data"}, FunctionManagerImpl::nativeBase64Decode);
    registerNativeFunction("md4_bin", {"data"}, FunctionManagerImpl::nativeMd4);
    registerNativeFunction("md4", {"data"}, FunctionManagerImpl::nativeMd4Hex);
    registerNativeFunction("md5_bin", {"data"}, FunctionManagerImpl::nativeMd5);
    registerNativeFunction("md5", {"data"}, FunctionManagerImpl::nativeMd5Hex);
    registerNativeFunction("sha1", {"data"}, FunctionManagerImpl::nativeSha1);
    registerNativeFunction("sha224", {"data"}, FunctionManagerImpl::nativeSha224);
    registerNativeFunction("sha256", {"data"}, FunctionManagerImpl::nativeSha256);
    registerNativeFunction("sha384", {"data"}, FunctionManagerImpl::nativeSha384);
    registerNativeFunction("sha512", {"data"}, FunctionManagerImpl::nativeSha512);
    registerNativeFunction("sha3_224", {"data"}, FunctionManagerImpl::nativeSha3_224);
    registerNativeFunction("sha3_256", {"data"}, FunctionManagerImpl::nativeSha3_256);
    registerNativeFunction("sha3_384", {"data"}, FunctionManagerImpl::nativeSha3_384);
    registerNativeFunction("sha3_512", {"data"}, FunctionManagerImpl::nativeSha3_512);
}

void FunctionManagerImpl::refreshFunctionsByKey()
{
    functionsByKey.clear();
    foreach (ScriptFunction* func, functions)
        functionsByKey[Key(func)] = func;

    foreach (NativeFunction* func, nativeFunctions)
        nativeFunctionsByKey[Key(func)] = func;
}

void FunctionManagerImpl::storeInConfig()
{
    QVariantList list;
    QHash<QString,QVariant> fnHash;
    foreach (ScriptFunction* func, functions)
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
    clearFunctions();

    QVariantList list = CFG_CORE.Internal.Functions.get();
    QHash<QString,QVariant> fnHash;
    ScriptFunction* func;
    for (const QVariant& var : list)
    {
        fnHash = var.toHash();
        func = new ScriptFunction();
        func->name = fnHash["name"].toString();
        func->lang = fnHash["lang"].toString();
        func->code = fnHash["code"].toString();
        func->initCode = fnHash["initCode"].toString();
        func->finalCode = fnHash["finalCode"].toString();
        func->databases = fnHash["databases"].toStringList();
        func->arguments = fnHash["arguments"].toStringList();
        func->type = static_cast<ScriptFunction::Type>(fnHash["type"].toInt());
        func->undefinedArgs = fnHash["undefinedArgs"].toBool();
        func->allDatabases = fnHash["allDatabases"].toBool();
        functions << func;
    }
}

void FunctionManagerImpl::clearFunctions()
{
    for (ScriptFunction* fn : functions)
        delete fn;

    functions.clear();
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

QVariant FunctionManagerImpl::nativeRegExp(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 2)
    {
        ok = false;
        return QVariant();
    }

    QRegularExpression re(args[0].toString());
    if (!re.isValid())
    {
        ok = false;
        return tr("Invalid regular expression pattern: %1").arg(args[0].toString());
    }

    QRegularExpressionMatch match = re.match(args[1].toString());
    return match.hasMatch();
}

QVariant FunctionManagerImpl::nativeSqlFile(const QList<QVariant>& args, Db* db, bool& ok)
{
    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    QFile file(args[0].toString());
    if (!file.open(QIODevice::ReadOnly))
    {
        ok = false;
        return tr("Could not open file %1 for reading: %2").arg(args[0].toString(), file.errorString());
    }

    QTextStream stream(&file);
    QString sql = stream.readAll();
    file.close();

    QueryExecutor executor(db);
    executor.setAsyncMode(false);
    executor.exec(sql);
    SqlQueryPtr results = executor.getResults();
    if (results->isError())
    {
        ok = false;
        return results->getErrorText();
    }
    return results->getSingleCell();
}

QVariant FunctionManagerImpl::nativeReadFile(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    QFile file(args[0].toString());
    if (!file.open(QIODevice::ReadOnly))
    {
        ok = false;
        return tr("Could not open file %1 for reading: %2").arg(args[0].toString(), file.errorString());
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

QVariant FunctionManagerImpl::nativeWriteFile(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 2)
    {
        ok = false;
        return QVariant();
    }

    QFile file(args[0].toString());
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        ok = false;
        return tr("Could not open file %1 for writting: %2").arg(args[0].toString(), file.errorString());
    }

    QByteArray data;
    switch (args[1].type())
    {
        case QVariant::String:
            data = args[1].toString().toLocal8Bit();
            break;
        default:
            data = args[1].toByteArray();
            break;
    }

    int res = file.write(data);
    file.close();

    if (res < 0)
    {
        ok = false;
        return tr("Error while writting to file %1: %2").arg(args[0].toString(), file.errorString());
    }

    return res;
}

QVariant FunctionManagerImpl::nativeScript(const QList<QVariant>& args, Db* db, bool& ok)
{
    if (args.size() != 2)
    {
        ok = false;
        return QVariant();
    }

    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(args[0].toString());
    if (!plugin)
    {
        ok = false;
        return tr("Unsupported scripting language: %1").arg(args[0].toString());
    }
    DbAwareScriptingPlugin* dbAwarePlugin = dynamic_cast<DbAwareScriptingPlugin*>(plugin);

    QString error;
    QVariant result;

    if (dbAwarePlugin)
        result = dbAwarePlugin->evaluate(args[1].toString(), QList<QVariant>(), db, false, &error);
    else
        result = plugin->evaluate(args[1].toString(), QList<QVariant>(), &error);

    if (!error.isEmpty())
    {
        ok = false;
        return error;
    }
    return result;
}

QVariant FunctionManagerImpl::nativeLangs(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 0)
    {
        ok = false;
        return QVariant();
    }

    QStringList names;
    for (ScriptingPlugin* plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        names << plugin->getLanguage();

    return names.join(", ");
}

QVariant FunctionManagerImpl::nativeHtmlEscape(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    return args[0].toString().toHtmlEscaped();
}

QVariant FunctionManagerImpl::nativeUrlEncode(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    return QUrl::toPercentEncoding(args[0].toString());
}

QVariant FunctionManagerImpl::nativeUrlDecode(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    return QUrl::fromPercentEncoding(args[0].toString().toLocal8Bit());
}

QVariant FunctionManagerImpl::nativeBase64Encode(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    return args[0].toByteArray().toBase64();
}

QVariant FunctionManagerImpl::nativeBase64Decode(const QList<QVariant>& args, Db* db, bool& ok)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    return QByteArray::fromBase64(args[0].toByteArray());
}

QVariant FunctionManagerImpl::nativeCryptographicFunction(const QList<QVariant>& args, Db* db, bool& ok, QCryptographicHash::Algorithm algo)
{
    UNUSED(db);

    if (args.size() != 1)
    {
        ok = false;
        return QVariant();
    }

    return QCryptographicHash::hash(args[0].toByteArray(), algo);
}

QVariant FunctionManagerImpl::nativeMd4(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Md4);
}

QVariant FunctionManagerImpl::nativeMd4Hex(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Md4).toByteArray().toHex();
}

QVariant FunctionManagerImpl::nativeMd5(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Md5);
}

QVariant FunctionManagerImpl::nativeMd5Hex(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Md5).toByteArray().toHex();
}

QVariant FunctionManagerImpl::nativeSha1(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha1);
}

QVariant FunctionManagerImpl::nativeSha224(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha224);
}

QVariant FunctionManagerImpl::nativeSha256(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha256);
}

QVariant FunctionManagerImpl::nativeSha384(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha384);
}

QVariant FunctionManagerImpl::nativeSha512(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha512);
}

QVariant FunctionManagerImpl::nativeSha3_224(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha3_224);
}

QVariant FunctionManagerImpl::nativeSha3_256(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha3_256);
}

QVariant FunctionManagerImpl::nativeSha3_384(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha3_384);
}

QVariant FunctionManagerImpl::nativeSha3_512(const QList<QVariant>& args, Db* db, bool& ok)
{
    return nativeCryptographicFunction(args, db, ok, QCryptographicHash::Sha3_512);
}

QStringList FunctionManagerImpl::getArgMarkers(int argCount)
{
    QStringList argMarkers;
    for (int i = 0; i < argCount; i++)
        argMarkers << "?";

    return argMarkers;
}

void FunctionManagerImpl::registerNativeFunction(const QString& name, const QStringList& args, FunctionManager::NativeFunction::ImplementationFunction funcPtr)
{
    NativeFunction* nf = new NativeFunction();
    nf->name = name;
    nf->arguments = args;
    nf->type = FunctionBase::SCALAR;
    nf->undefinedArgs = false;
    nf->functionPtr = funcPtr;
    nativeFunctions << nf;
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

FunctionManagerImpl::Key::Key(FunctionBase* function) :
    name(function->name), argCount(function->undefinedArgs ? -1 : function->arguments.size()), type(function->type)
{
}
