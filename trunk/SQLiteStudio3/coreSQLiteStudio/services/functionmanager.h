#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include <QList>
#include <QSharedPointer>
#include <QObject>
#include <QStringList>

class Db;

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

        virtual void setFunctions(const QList<FunctionPtr>& newFunctions) = 0;
        virtual QList<FunctionPtr> getAllFunctions() const = 0;
        virtual QList<FunctionPtr> getFunctionsForDatabase(const QString& dbName) const = 0;
        virtual QVariant evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok) = 0;
        virtual void evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage) = 0;
        virtual void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage) = 0;
        virtual QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage) = 0;

    signals:
        void functionListChanged();
};

#define FUNCTIONS SQLITESTUDIO->getFunctionManager()

#endif // FUNCTIONMANAGER_H
