#ifndef SCRIPTINGSQL_H
#define SCRIPTINGSQL_H

#include "genericplugin.h"
#include "scriptingplugin.h"

class ScriptingSql : public GenericPlugin, public DbAwareScriptingPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_TITLE("SQL scripting")
    SQLITESTUDIO_PLUGIN_DESC("SQL scripting support.")
    SQLITESTUDIO_PLUGIN_VERSION(10000)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        class SqlContext : public Context
        {
            public:
                QString errorText;
                QHash<QString,QVariant> variables;
        };

        ScriptingSql();
        ~ScriptingSql();

        QString getLanguage() const;
        Context* createContext();
        void releaseContext(Context* context);
        void resetContext(Context* context);
        QVariant evaluate(Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking);
        QVariant evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage);
        void setVariable(Context* context, const QString& name, const QVariant& value);
        QVariant getVariable(Context* context, const QString& name);
        bool hasError(Context* context) const;
        QString getErrorMessage(Context* context) const;
        QByteArray getIconData() const;
        bool init();
        void deinit();

    private:
        QList<Context*> contexts;
        Db* memDb = nullptr;
};

#endif // SCRIPTINGSQL_H
