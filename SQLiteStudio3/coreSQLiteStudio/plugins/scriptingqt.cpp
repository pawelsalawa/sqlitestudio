#include "scriptingqt.h"
#include "common/unused.h"
#include "common/global.h"
#include "scriptingqtdbproxy.h"
#include "services/notifymanager.h"
#include <QJSEngine>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

ScriptingQt::ScriptingQt()
{
    mainEngineMutex = new QMutex();
}

ScriptingQt::~ScriptingQt()
{
    safe_delete(mainEngineMutex);
}

QJSValueList ScriptingQt::toValueList(QJSEngine* engine, const QList<QVariant>& values)
{
    QJSValueList result;
    for (const QVariant& value : values)
        result << engine->toScriptValue(value);

    return result;
}

QString ScriptingQt::getLanguage() const
{
    return QStringLiteral("JavaScript");
}

ScriptingPlugin::Context* ScriptingQt::createContext()
{
    ContextQt* ctx = new ContextQt;
//    ctx->engine->pushContext();
    contexts << ctx;
    return ctx;
}

void ScriptingQt::releaseContext(ScriptingPlugin::Context* context)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return;

    contexts.removeOne(ctx);
    delete ctx;
}

void ScriptingQt::resetContext(ScriptingPlugin::Context* context)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return;
}

QVariant ScriptingQt::evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
{
    QMutexLocker locker(mainEngineMutex);

    // Call the function
    QVariant result = evaluate(mainContext, code, args, db, locking);

    // Handle errors
    if (!mainContext->error.isEmpty())
        *errorMessage = mainContext->error;

    return result;
}

QVariant ScriptingQt::evaluate(ScriptingPlugin::Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return evaluate(ctx, code, args, db, locking);
}

QVariant ScriptingQt::evaluate(ContextQt* ctx, const QString& code, const QList<QVariant>& args, Db* db, bool locking)
{
    // Define function to call
    QJSValue functionValue = getFunctionValue(ctx, code);

    // Db for this evaluation
    ctx->dbProxy->setDb(db);
    ctx->dbProxy->setUseDbLocking(locking);

    // Call the function
    QJSValue result;
    if (args.size() > 0)
        result = functionValue.call(toValueList(ctx->engine, args));
    else
        result = functionValue.call();

    // Handle errors
    ctx->error.clear();
    if (result.isError())
    {
        ctx->error = QString("Uncaught exception at line %1: %2").arg(
                        result.property("lineNumber").toString(),
                        result.toString());
    }

    ctx->dbProxy->setDb(nullptr);
    ctx->dbProxy->setUseDbLocking(false);

    return convertVariant(result.toVariant());
}

QVariant ScriptingQt::convertVariant(const QVariant& value, bool wrapStrings)
{
    switch (value.type())
    {
        case QVariant::Hash:
        {
            QHash<QString, QVariant> hash = value.toHash();
            QHashIterator<QString, QVariant> it(hash);
            QStringList list;
            while (it.hasNext())
            {
                it.next();
                list << it.key() + ": " + convertVariant(it.value(), true).toString();
            }
            return "{" + list.join(", ") + "}";
        }
        case QVariant::Map:
        {
            QMap<QString, QVariant> map = value.toMap();
            QMapIterator<QString, QVariant> it(map);
            QStringList list;
            while (it.hasNext())
            {
                it.next();
                list << it.key() + ": " + convertVariant(it.value(), true).toString();
            }
            return "{" + list.join(", ") + "}";
        }
        case QVariant::List:
        {
            QStringList list;
            for (const QVariant& var : value.toList())
                list << convertVariant(var, true).toString();

            return "[" + list.join(", ") + "]";
        }
        case QVariant::StringList:
        {
            return "[\"" + value.toStringList().join("\", \"") + "\"]";
        }
        case QVariant::String:
        {
            if (wrapStrings)
                return "\"" + value.toString() + "\"";

            break;
        }
        default:
            break;
    }
    return value;
}

void ScriptingQt::setVariable(ScriptingPlugin::Context* context, const QString& name, const QVariant& value)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return;

    ctx->engine->globalObject().setProperty(name, ctx->engine->toScriptValue(value));
}

QVariant ScriptingQt::getVariable(ScriptingPlugin::Context* context, const QString& name)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    QJSValue value = ctx->engine->globalObject().property(name);
    return convertVariant(value.toVariant());
}

bool ScriptingQt::hasError(ScriptingPlugin::Context* context) const
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return false;

    return !ctx->error.isEmpty();
}

QString ScriptingQt::getErrorMessage(ScriptingPlugin::Context* context) const
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QString();

    return ctx->error;
}

QString ScriptingQt::getIconPath() const
{
    return ":/images/plugins/scriptingqt.png";
}

bool ScriptingQt::init()
{
    QMutexLocker locker(mainEngineMutex);
    mainContext = new ContextQt;
    return true;
}

void ScriptingQt::deinit()
{
    for (Context* ctx : contexts)
        delete ctx;

    contexts.clear();

    QMutexLocker locker(mainEngineMutex);
    safe_delete(mainContext);
}

ScriptingQt::ContextQt* ScriptingQt::getContext(ScriptingPlugin::Context* context) const
{
    ContextQt* ctx = dynamic_cast<ContextQt*>(context);
    if (!ctx)
        qDebug() << "Invalid context passed to ScriptingQt:" << context;

    return ctx;
}

QJSValue ScriptingQt::getFunctionValue(ContextQt* ctx, const QString& code)
{
    static const QString fnDef = QStringLiteral("(function () {%1\n})");

    QJSValue* func = nullptr;
    if (!ctx->scriptCache.contains(code))
    {
        func = new QJSValue(ctx->engine->evaluate(fnDef.arg(code)));
        ctx->scriptCache.insert(code, func);
    }
    else
    {
        func = ctx->scriptCache[code];
    }
    return *func;
}

ScriptingQt::ContextQt::ContextQt()
{
    engine = new QJSEngine();
    engine->installExtensions(QJSEngine::ConsoleExtension);

    dbProxy = new ScriptingQtDbProxy(engine);
    dbProxyScriptValue = engine->newQObject(dbProxy);
    debugger = new ScriptingQtDebugger(engine);

    engine->globalObject().setProperty("$$debug", engine->newQObject(debugger));
    engine->globalObject().setProperty("debug", engine->globalObject().property("$$debug").property("debug"));
    engine->globalObject().setProperty("db", dbProxyScriptValue);

    scriptCache.setMaxCost(cacheSize);
}

ScriptingQt::ContextQt::~ContextQt()
{
    safe_delete(debugger);
    safe_delete(dbProxy);
    safe_delete(engine);
}

ScriptingQtDebugger::ScriptingQtDebugger(QJSEngine* engine) :
    QObject(), engine(engine)
{
}

QJSValue ScriptingQtDebugger::debug(const QVariant& value)
{
    NOTIFY_MANAGER->info("[ScriptingQt] " + value.toString());
    return QJSValue();
}
