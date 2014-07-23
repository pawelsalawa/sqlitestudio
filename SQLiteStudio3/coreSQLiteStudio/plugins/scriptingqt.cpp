#include "scriptingqt.h"
#include "common/unused.h"
#include "common/global.h"
#include "scriptingqtdbproxy.h"
#include <QScriptEngine>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

static QScriptValue scriptingQtDebug(QScriptContext *context, QScriptEngine *engine)
{
    UNUSED(engine);
    QStringList args;
    for (int i = 0; i < context->argumentCount(); i++)
        args << context->argument(i).toString();

    qDebug() << "[ScriptingQt]" << args;
    return QScriptValue();
}

ScriptingQt::ScriptingQt()
{
    mainEngineMutex = new QMutex();
}

ScriptingQt::~ScriptingQt()
{
    safe_delete(mainEngineMutex);
}

QString ScriptingQt::getLanguage() const
{
    return QStringLiteral("QtScript");
}

ScriptingPlugin::Context* ScriptingQt::createContext()
{
    ContextQt* ctx = new ContextQt;
    ctx->engine->pushContext();
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

    ctx->engine->popContext();
    ctx->engine->pushContext();
}

QVariant ScriptingQt::evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
{
    QMutexLocker locker(mainEngineMutex);

    // Enter a new context
    QScriptContext* engineContext = mainContext->engine->pushContext();

    // Call the function
    QVariant result = evaluate(mainContext, engineContext, code, args, db, locking);

    // Handle errors
    if (!mainContext->error.isEmpty())
        *errorMessage = mainContext->error;

    // Leave the context to reset "this".
    mainContext->engine->popContext();

    return result;
}

QVariant ScriptingQt::evaluate(ScriptingPlugin::Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return evaluate(ctx, ctx->engine->currentContext(), code, args, db, locking);
}

QVariant ScriptingQt::evaluate(ContextQt* ctx, QScriptContext* engineContext, const QString& code, const QList<QVariant>& args, Db* db, bool locking)
{
    // Define function to call
    QScriptValue functionValue = getFunctionValue(ctx, code);

    // Db for this evaluation
    ctx->dbProxy->setDb(db);
    ctx->dbProxy->setUseDbLocking(locking);

    // Call the function
    QScriptValue result;
    if (args.size() > 0)
        result = functionValue.call(engineContext->activationObject(), ctx->engine->toScriptValue(args));
    else
        result = functionValue.call(engineContext->activationObject());

    // Handle errors
    ctx->error.clear();
    if (ctx->engine->hasUncaughtException())
        ctx->error = ctx->engine->uncaughtException().toString();

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

    ctx->engine->globalObject().setProperty(name, ctx->engine->newVariant(value));
}

QVariant ScriptingQt::getVariable(ScriptingPlugin::Context* context, const QString& name)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    QScriptValue value = ctx->engine->globalObject().property(name);
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
        return QString::null;

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
    foreach (Context* ctx, contexts)
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

QScriptValue ScriptingQt::getFunctionValue(ContextQt* ctx, const QString& code)
{
    static const QString fnDef = QStringLiteral("(function () {%1\n})");

    QScriptProgram* prog = nullptr;
    if (!ctx->scriptCache.contains(code))
    {
        prog = new QScriptProgram(fnDef.arg(code));
        ctx->scriptCache.insert(code, prog);
    }
    else
    {
        prog = ctx->scriptCache[code];
    }
    return ctx->engine->evaluate(*prog);
}

ScriptingQt::ContextQt::ContextQt()
{
    engine = new QScriptEngine();

    dbProxy = new ScriptingQtDbProxy();
    dbProxyScriptValue = engine->newQObject(dbProxy, QScriptEngine::QtOwnership, QScriptEngine::ExcludeDeleteLater);

    engine->globalObject().setProperty("debug", engine->newFunction(scriptingQtDebug));
    engine->globalObject().setProperty("db", dbProxyScriptValue);

    scriptCache.setMaxCost(cacheSize);
}

ScriptingQt::ContextQt::~ContextQt()
{
    safe_delete(engine);
    safe_delete(dbProxy);
}
