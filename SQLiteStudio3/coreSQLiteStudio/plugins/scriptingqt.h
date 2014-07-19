#ifndef SCRIPTINGQT_H
#define SCRIPTINGQT_H

#include "builtinplugin.h"
#include "scriptingplugin.h"
#include <QHash>
#include <QVariant>
#include <QCache>
#include <QScriptValue>
#include <QScriptProgram>

class QScriptEngine;
class QMutex;
class QScriptContext;

class ScriptingQt : public BuiltInPlugin, public ScriptingPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_TITLE("Qt scripting")
    SQLITESTUDIO_PLUGIN_DESC("Qt scripting support.")
    SQLITESTUDIO_PLUGIN_VERSION(10000)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        ScriptingQt();
        ~ScriptingQt();

        QString getLanguage() const;
        Context* createContext();
        void releaseContext(Context* context);
        void resetContext(Context* context);
        QVariant evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage = nullptr);
        QVariant evaluate(Context* context, const QString& code, const QList<QVariant>& args);
        void setVariable(Context* context, const QString& name, const QVariant& value);
        QVariant getVariable(Context* context, const QString& name);
        bool hasError(Context* context) const;
        QString getErrorMessage(Context* context) const;
        QString getIconPath() const;
        bool init();
        void deinit();

    private:
        class ContextQt : public ScriptingPlugin::Context
        {
            public:
                ContextQt();
                ~ContextQt();

                QScriptEngine* engine;
                QCache<QString,QScriptProgram> scriptCache;
                QString error;
        };

        ContextQt* getContext(ScriptingPlugin::Context* context) const;
        QScriptValue getFunctionValue(ContextQt* ctx, const QString& code);
        QVariant evaluate(ContextQt* ctx, QScriptContext* engineContext, const QString& code, const QList<QVariant>& args);
        QVariant convertList(const QVariant& value);

        static const constexpr int cacheSize = 5;

        ContextQt* mainContext;
        QList<Context*> contexts;
        QMutex* mainEngineMutex;
};

#endif // SCRIPTINGQT_H
