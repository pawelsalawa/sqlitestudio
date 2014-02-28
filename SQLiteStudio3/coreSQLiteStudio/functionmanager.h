#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include "strhash.h"
#include "sqlitestudio.h"
#include <QList>
#include <QSharedPointer>
#include <QObject>

class Db;

class FunctionManager : public QObject
{
    Q_OBJECT

    public:
        struct Function
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
        FunctionPtr getFunction(const QString& name);
        QVariant evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok);
        void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db);
        QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok);

    private:
        void init();
        void refreshFunctionsByName();
        void storeInConfig();

        QList<FunctionPtr> functions;
        StrHash<FunctionPtr> functionsByName;

    signals:
        void functionListChanged();
};

#define FUNCTIONS SQLiteStudio::getInstance()->getFunctionManager()

#endif // FUNCTIONMANAGER_H
