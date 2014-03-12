#ifndef SQLFUNCTIONQT_H
#define SQLFUNCTIONQT_H

#include "sqlfunctionqt_global.h"
#include "plugins/genericplugin.h"
#include "plugins/sqlfunctionplugin.h"

class ScriptingPlugin;

class SQLFUNCTIONQTSHARED_EXPORT SqlFunctionQt : public GenericPlugin, public SqlFunctionPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN
    SQLITESTUDIO_PLUGIN_TITLE("Qt functions")
    SQLITESTUDIO_PLUGIN_DESC("Supports custom query functions implemented with Qt scripting (JavaScript-like) language.")
    SQLITESTUDIO_PLUGIN_VERSION(10000)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        QVariant evaluateScalar(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, bool& success);
        void evaluateAggregateInitial(Db* db, const QString& function, int argCount, const QString& code, QHash<QString, QVariant>& aggregateStorage);
        void evaluateAggregateStep(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateAggregateFinal(Db* db, const QString& function, int argCount, const QString& code, bool& success, QHash<QString, QVariant>& aggregateStorage);
        QString getLanguageName() const;
        QByteArray getIconData() const;

    private:
        ScriptingPlugin* getEngine();

        ScriptingPlugin* scriptingEngine = nullptr;
};

#endif // SQLFUNCTIONQT_H
