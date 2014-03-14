#ifndef FUNCTIONMANAGERIMPL_H
#define FUNCTIONMANAGERIMPL_H

#include "services/functionmanager.h"

class SqlFunctionPlugin;
class Plugin;
class PluginType;

class FunctionManagerImpl : public FunctionManager
{
    Q_OBJECT

    public:
        FunctionManagerImpl();

        void setFunctions(const QList<FunctionPtr>& newFunctions);
        QList<FunctionPtr> getAllFunctions() const;
        QList<FunctionPtr> getFunctionsForDatabase(const QString& dbName) const;
        QVariant evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok);
        void evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage);
        void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage);

    private:
        struct Key
        {
            Key();
            Key(FunctionPtr function);

            QString name;
            int argCount;
            Function::Type type;
        };

        friend int qHash(const FunctionManagerImpl::Key& key);
        friend bool operator==(const FunctionManagerImpl::Key& key1, const FunctionManagerImpl::Key& key2);

        void init();
        void refreshFunctionsByKey();
        void storeInConfig();
        QString cannotFindFunctionError(const QString& name, int argCount);
        QString langUnsupportedError(const QString& name, int argCount, const QString& lang);
        static QStringList getArgMarkers(int argCount);

        QList<FunctionPtr> functions;
        QHash<Key,FunctionPtr> functionsByKey;
        QHash<QString,SqlFunctionPlugin*> functionPlugins;

    private slots:
        void pluginLoaded(Plugin* plugin, PluginType* type);
        void pluginUnloaded(Plugin* plugin, PluginType* type);
};

int qHash(const FunctionManagerImpl::Key& key);
bool operator==(const FunctionManagerImpl::Key& key1, const FunctionManagerImpl::Key& key2);

#endif // FUNCTIONMANAGERIMPL_H
