#include "scriptingqt.h"
#include <QScriptEngine>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

ScriptingQt::ScriptingQt()
{
    mainEngineMutex = new QMutex();
}

ScriptingQt::~ScriptingQt()
{
    if (mainEngineMutex)
    {
        delete mainEngineMutex;
        mainEngineMutex = nullptr;
    }
}

QString ScriptingQt::getLanguage() const
{
    return "Qt";
}

ScriptingPlugin::Context* ScriptingQt::createContext()
{
    ContextQt* ctx = new ContextQt;
    ctx->engine = new QScriptEngine();
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
    delete ctx->engine;
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

QVariant ScriptingQt::evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage) const
{
    QMutexLocker locker(mainEngineMutex);

    QScriptContext* engineContext = mainEngine->pushContext();

    if (args.size() > 0)
    {
        engineContext->activationObject().setProperty("args", mainEngine->newVariant(args));
    }
    else
    {
        engineContext->activationObject().setProperty("args", mainEngine->nullValue());
    }

    QScriptValue value = mainEngine->evaluate(code);
    if (errorMessage && mainEngine->hasUncaughtException())
    {
        *errorMessage = mainEngine->uncaughtException().toString() + "\n";
        *errorMessage += mainEngine->uncaughtExceptionBacktrace().join("\n");
    }

    mainEngine->popContext();
    return value.toVariant();
}

QVariant ScriptingQt::evaluate(ScriptingPlugin::Context* context, const QString& code) const
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    QScriptValue value = ctx->engine->evaluate(code);
    if (mainEngine->hasUncaughtException())
    {
        ctx->error = mainEngine->uncaughtException().toString() + "\n";
        ctx->error += mainEngine->uncaughtExceptionBacktrace().join("\n");
    }
    else
    {
        ctx->error.clear();
    }

    return value.toVariant();
}

void ScriptingQt::setVariable(ScriptingPlugin::Context* context, const QString& name, const QVariant& value) const
{
    ContextQt* ctx = getContext(context);
    if (!ctx)
        return;

    ctx->engine->globalObject().setProperty(name, ctx->engine->newVariant(value));
}

QVariant ScriptingQt::getVariable(ScriptingPlugin::Context* context, const QString& name) const
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

bool ScriptingQt::init()
{
    QMutexLocker locker(mainEngineMutex);
    mainEngine = new QScriptEngine();
    return true;
}

void ScriptingQt::deinit()
{
    foreach (Context* ctx, contexts)
    {
        delete dynamic_cast<ContextQt*>(ctx)->engine;
        delete ctx;
    }
    contexts.clear();

    QMutexLocker locker(mainEngineMutex);
    if (mainEngine)
    {
        delete mainEngine;
        mainEngine = nullptr;
    }
}

ScriptingQt::ContextQt* ScriptingQt::getContext(ScriptingPlugin::Context* context) const
{
    ContextQt* ctx = dynamic_cast<ContextQt*>(context);
    if (!ctx)
        qDebug() << "Invalid context passed to ScriptingQt:" << context;

    return ctx;
}
