#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include "strhash.h"
#include "sqlitestudio.h"
#include <QList>
#include <QSharedPointer>
#include <QObject>

class Db;
class SqlFunctionPlugin;

class API_EXPORT FunctionManager : public QObject
{
    Q_OBJECT

    public:
        struct API_EXPORT Function
        {
            enum Type
            {
                SCALAR = 0,
                AGGREGATE = 1
            };

            Function();

            static QString typeString(Type type);
            static Type typeString(const QString& type);

            QString name;
            QString lang;
            QString code;
            QString initCode;
            QString finalCode;
            QStringList databases;
            QStringList arguments;
            Type type = SCALAR;
            bool undefinedArgs = true;
            bool allDatabases = true;
        };

        typedef QSharedPointer<Function> FunctionPtr;

        FunctionManager();
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

        friend int qHash(const FunctionManager::Key& key);
        friend bool operator==(const FunctionManager::Key& key1, const FunctionManager::Key& key2);

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

    signals:
        void functionListChanged();
};

int qHash(const FunctionManager::Key& key);
bool operator==(const FunctionManager::Key& key1, const FunctionManager::Key& key2);

#define FUNCTIONS SQLiteStudio::getInstance()->getFunctionManager()

#endif // FUNCTIONMANAGER_H
