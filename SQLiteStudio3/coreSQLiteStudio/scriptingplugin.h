#ifndef SCRIPTINGPLUGIN_H
#define SCRIPTINGPLUGIN_H

#include "plugin.h"
#include <QVariant>

class ScriptingPlugin : virtual public Plugin
{
    public:
        class Context
        {
            public:
                virtual ~Context() {}
        };

        virtual QString getLanguage() const = 0;
        virtual Context* createContext() = 0;
        virtual void releaseContext(Context* context) = 0;
        virtual void resetContext(Context* context) = 0;
        virtual void setVariable(Context* context, const QString& name, const QVariant& value) const = 0;
        virtual QVariant getVariable(Context* context, const QString& name) const = 0;
        virtual QVariant evaluate(Context* context, const QString& code) const = 0;
        virtual bool hasError(Context* context) const = 0;
        virtual QString getErrorMessage(Context* context) const = 0;
        virtual QVariant evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage = nullptr) const = 0;
};

Q_DECLARE_METATYPE(ScriptingPlugin::Context*)

#endif // SCRIPTINGPLUGIN_H
