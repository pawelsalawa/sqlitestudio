#include "scriptingqt.h"
#include "common/unused.h"
#include "common/global.h"
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
    ctx->scriptCache.setMaxCost(cacheSize);
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

QVariant ScriptingQt::evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage)
{
    QMutexLocker locker(mainEngineMutex);

    // Enter a new context
    QScriptContext* engineContext = mainContext->engine->pushContext();

    // Call the function
    QVariant result = evaluate(mainContext, engineContext, code, args);

    // Handle errors
    if (!mainContext->error.isEmpty())
        *errorMessage = mainContext->error;

    // Leave the context to reset "this".
    mainContext->engine->popContext();

    return result;
}

QVariant ScriptingQt::evaluate(ScriptingPlugin::Context* context, const QString& code, const QList<QVariant>& args)
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return evaluate(ctx, ctx->engine->currentContext(), code, args);
}

QVariant ScriptingQt::evaluate(ContextQt* ctx, QScriptContext* engineContext, const QString& code, const QList<QVariant>& args)
{
    // Define function to call
    QScriptValue functionValue = getFunctionValue(ctx, code);

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

    return result.toVariant();
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
    return value.toVariant();
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

QByteArray ScriptingQt::getIconData() const
{
    static_char* icon = "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";

    return QByteArray(icon);
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
    engine->globalObject().setProperty("debug", engine->newFunction(scriptingQtDebug));
}

ScriptingQt::ContextQt::~ContextQt()
{
    safe_delete(engine);
}
