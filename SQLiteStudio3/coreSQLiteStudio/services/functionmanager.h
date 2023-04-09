#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include "coreSQLiteStudio_global.h"
#include <QVariant>
#include <QList>
#include <QSharedPointer>
#include <QObject>
#include <QStringList>
#include <functional>

class Db;

class API_EXPORT FunctionManager : public QObject
{
    Q_OBJECT

    public:
        struct API_EXPORT FunctionBase
        {
            enum Type
            {
                SCALAR = 0,
                AGGREGATE = 1
            };

            FunctionBase();
            virtual ~FunctionBase();

            virtual QString toString() const;

            static QString typeString(Type type);
            static Type typeString(const QString& type);

            QString name;
            QStringList arguments;
            Type type = SCALAR;
            bool undefinedArgs = true;
            bool deterministic = false;
        };

        struct API_EXPORT ScriptFunction : public FunctionBase
        {
            QString lang;
            QString code;
            QString initCode;
            QString finalCode;
            QStringList databases;
            bool allDatabases = true;
        };

        struct API_EXPORT NativeFunction : public FunctionBase
        {
            typedef std::function<QVariant(const QList<QVariant>& args, Db* db, bool& ok)> ImplementationFunction;

            ImplementationFunction functionPtr;
        };

        virtual void setScriptFunctions(const QList<ScriptFunction*>& newFunctions) = 0;
        virtual QList<ScriptFunction*> getAllScriptFunctions() const = 0;
        virtual QList<ScriptFunction*> getScriptFunctionsForDatabase(const QString& dbName) const = 0;
        virtual QList<NativeFunction*> getAllNativeFunctions() const = 0;

        virtual QVariant evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok) = 0;
        virtual void evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage) = 0;
        virtual void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db,
                                           QHash<QString, QVariant>& aggregateStorage) = 0;
        virtual QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage) = 0;

    signals:
        void functionListChanged();
};

#define FUNCTIONS SQLITESTUDIO->getFunctionManager()

#endif // FUNCTIONMANAGER_H
