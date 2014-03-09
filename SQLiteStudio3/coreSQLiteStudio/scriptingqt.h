#ifndef SCRIPTINGQT_H
#define SCRIPTINGQT_H

#include "genericplugin.h"
#include "scriptingplugin.h"
#include <QHash>
#include <QVariant>

class QScriptEngine;
class QMutex;

class ScriptingQt : public GenericPlugin, public ScriptingPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN
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
        QVariant evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage = nullptr) const;
        QVariant evaluate(Context* context, const QString& code) const;
        void setVariable(Context* context, const QString& name, const QVariant& value) const;
        QVariant getVariable(Context* context, const QString& name) const;
        bool hasError(Context* context) const;
        QString getErrorMessage(Context* context) const;
        bool init();
        void deinit();

    private:
        class ContextQt : public ScriptingPlugin::Context
        {
            public:
                QScriptEngine* engine;
                QString error;
        };

        ContextQt* getContext(ScriptingPlugin::Context* context) const;

        QScriptEngine* mainEngine;
        QList<Context*> contexts;
        QMutex* mainEngineMutex;
};

#endif // SCRIPTINGQT_H
