#include "scriptingqt.h"
#include "common/global.h"
#include "scriptingqtdbproxy.h"
#include "services/notifymanager.h"
#include <QJSEngine>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

ScriptingQt::ScriptingQt()
{
    managedMainContextsMutex = new QMutex();
}

ScriptingQt::~ScriptingQt()
{
    safe_delete(managedMainContextsMutex);
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

QVariant ScriptingQt::evaluate(const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
{
    // Call the function
    ContextQt* context = getMainContext();
    QVariant result = evaluate(context, code, funcInfo, args, db, locking);

    // Handle errors
    if (!context->error.isEmpty())
        *errorMessage = context->error;

    return result;
}

QVariant ScriptingQt::evaluate(ScriptingPlugin::Context* context, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return evaluate(ctx, code, funcInfo, args, db, locking);
}

QVariant ScriptingQt::evaluate(ContextQt* ctx, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking)
{
    // Define function to call
    QJSValue functionValue = getFunctionValue(ctx, code, funcInfo);

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

ScriptingQt::ContextQt* ScriptingQt::getMainContext()
{
    if (mainContext.hasLocalData())
        return mainContext.localData();

    ContextQt* context = new ContextQt();
    mainContext.setLocalData(context);

    QMutexLocker locker(managedMainContextsMutex);
    managedMainContexts << context;

    return context;
}

QVariant ScriptingQt::convertVariant(const QVariant& value, bool wrapStrings)
{
    switch (value.userType())
    {
        case QMetaType::QVariantHash:
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
        case QMetaType::QVariantMap:
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
        case QMetaType::QVariantList:
        {
            QStringList list;
            for (QVariant& var : value.toList())
                list << convertVariant(var, true).toString();

            return "[" + list.join(", ") + "]";
        }
        case QMetaType::QStringList:
        {
            return "[\"" + value.toStringList().join("\", \"") + "\"]";
        }
        case QMetaType::QString:
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
    return ":/images/plugins/scriptingqt.svg";
}

bool ScriptingQt::init()
{
    return true;
}

void ScriptingQt::deinit()
{
    for (Context*& ctx : contexts)
        delete ctx;

    contexts.clear();

    QMutexLocker locker(managedMainContextsMutex);
    for (ContextQt*& ctx : managedMainContexts)
    {
        ctx->engine->setInterrupted(true);
        delete ctx;
    }

    managedMainContexts.clear();
}

ScriptingQt::ContextQt* ScriptingQt::getContext(ScriptingPlugin::Context* context) const
{
    ContextQt* ctx = dynamic_cast<ContextQt*>(context);
    if (!ctx)
        qDebug() << "Invalid context passed to ScriptingQt:" << context;

    return ctx;
}

QJSValue ScriptingQt::getFunctionValue(ContextQt* ctx, const QString& code, const FunctionInfo& funcInfo)
{
    static const QString fnDef = QStringLiteral("(function (%1) {%2\n})");

    QString fullCode = fnDef.arg(funcInfo.getArguments().join(", "), code);
    QJSValue* func = ctx->scriptCache[fullCode];
    if (func)
        return *func;

    func = new QJSValue(ctx->engine->evaluate(fullCode));
    ctx->scriptCache.insert(fullCode, func);
    return *func;
}

ScriptingQt::ContextQt::ContextQt()
{
    engine = new QJSEngine();
    engine->installExtensions(QJSEngine::ConsoleExtension);

    dbProxy = new ScriptingQtDbProxy(engine);
    dbProxyScriptValue = engine->newQObject(dbProxy);
    console = new ScriptingQtConsole(engine);

    engine->globalObject().setProperty("console", engine->newQObject(console));
    engine->globalObject().setProperty("db", dbProxyScriptValue);

    scriptCache.setMaxCost(cacheSize);
}

ScriptingQt::ContextQt::~ContextQt()
{
    safe_delete(console);
    safe_delete(dbProxy);
    safe_delete(engine);
}

ScriptingQtConsole::ScriptingQtConsole(QJSEngine* engine) :
    QObject(), engine(engine)
{
}

QJSValue ScriptingQtConsole::log(const QJSValue& value)
{
    static_qstring(tpl, "[JS] %1");
    NOTIFY_MANAGER->info(tpl.arg(ScriptingQt::convertVariant(value.toVariant()).toString()));
    return QJSValue();
}
