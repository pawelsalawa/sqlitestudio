#include "sqlfunctionqt.h"
#include "scriptingplugin.h"
#include "pluginmanager.h"
#include "unused.h"

QVariant SqlFunctionQt::evaluateScalar(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, bool& success)
{
    UNUSED(db);
    UNUSED(function);

    QString error;
    QVariant result = getEngine()->evaluate(code, args, &error);
    if (!error.isEmpty())
    {
        success = false;
        return error;
    }
    return result;
}

void SqlFunctionQt::evaluateAggregateInitial(Db* db, const QString& function, int argCount, const QString& code, QHash<QString, QVariant>& aggregateStorage)
{
    UNUSED(db);
    UNUSED(function);
    UNUSED(argCount);
    UNUSED(code);

    ScriptingPlugin::Context* ctx = getEngine()->createContext();
    aggregateStorage["context"] = QVariant::fromValue(ctx);

    getEngine()->evaluate(ctx, code, {});

    if (getEngine()->hasError(ctx))
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorMessage"] = getEngine()->getErrorMessage(ctx);
    }
}

void SqlFunctionQt::evaluateAggregateStep(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, QHash<QString, QVariant>& aggregateStorage)
{
    UNUSED(db);
    UNUSED(function);

    if (aggregateStorage.contains("error"))
        return;

    ScriptingPlugin::Context* ctx = aggregateStorage["context"].value<ScriptingPlugin::Context*>();
    getEngine()->evaluate(ctx, code, args);

    if (getEngine()->hasError(ctx))
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorMessage"] = getEngine()->getErrorMessage(ctx);
    }
}

QVariant SqlFunctionQt::evaluateAggregateFinal(Db* db, const QString& function, int argCount, const QString& code, bool& success, QHash<QString, QVariant>& aggregateStorage)
{
    UNUSED(db);
    UNUSED(function);
    UNUSED(argCount);

    ScriptingPlugin::Context* ctx = aggregateStorage["context"].value<ScriptingPlugin::Context*>();
    if (aggregateStorage.contains("error"))
    {
        success = false;
        getEngine()->releaseContext(ctx);
        return aggregateStorage["errorMessage"];
    }

    QVariant result = getEngine()->evaluate(ctx, code, {});

    if (getEngine()->hasError(ctx))
    {
        success = false;
        QString msg = getEngine()->getErrorMessage(ctx);
        getEngine()->releaseContext(ctx);
        return msg;
    }

    getEngine()->releaseContext(ctx);
    return result;
}

QString SqlFunctionQt::getLanguageName() const
{
    return "Qt";
}

QByteArray SqlFunctionQt::getIconData() const
{
    static const char* icon = "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";

    return QByteArray(icon);
}

ScriptingPlugin*SqlFunctionQt::getEngine()
{
    if (scriptingEngine)
        return scriptingEngine;

    foreach (ScriptingPlugin* plugin, PLUGINS->getLoadedPlugins<ScriptingPlugin>())
    {
        if (plugin->getLanguage() == getLanguageName())
        {
            scriptingEngine = plugin;
            break;
        }
    }
    return scriptingEngine;
}
