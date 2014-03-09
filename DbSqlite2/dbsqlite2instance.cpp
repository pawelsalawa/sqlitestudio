#include "dbsqlite2instance.h"

DbSqlite2Instance::DbSqlite2Instance(const QString &driverName, const QString &type)
    : DbQt2<Sqlite2>(driverName, type)
{
}

//void DbSqlite2Instance::interruptExecutionOnHandle(const QVariant &handle)
//{
//    sqlite* sqliteHnd = getHandle(handle);
//    if (!sqliteHnd)
//        return;

//    sqlite_interrupt(sqliteHnd);
//}

//bool DbSqlite2Instance::deregisterFunction(const QVariant& handle, const QString& name, int argCount)
//{
//    sqlite* sqliteHnd = getHandle(handle);
//    if (!sqliteHnd)
//        return false;

//    sqlite_create_function(sqliteHnd, name.toLatin1().data(), argCount, nullptr, nullptr);
//    sqlite_create_aggregate(sqliteHnd, name.toLatin1().data(), argCount, nullptr, nullptr, nullptr);

//    FunctionUserData* userData = nullptr;
//    QMutableListIterator<FunctionUserData*> it(userDataList);
//    while (it.hasNext())
//    {
//        userData = it.next();
//        if (userData->name == name && userData->argCount == argCount)
//        {
//            it.remove();
//            delete userData;
//        }
//    }

//    return true;
//}

//bool DbSqlite2Instance::registerScalarFunction(const QVariant& handle, const QString& name, int argCount)
//{
//    sqlite* sqliteHnd = getHandle(handle);
//    if (!sqliteHnd)
//        return false;

//    FunctionUserData* userData = new FunctionUserData;
//    userData->db = this;
//    userData->name = name;
//    userData->argCount = argCount;

//    int res = sqlite_create_function(sqliteHnd, name.toLatin1().data(), argCount,
//                                     &DbSqlite2Instance::evaluateScalar, userData);

//    return res == SQLITE_OK;
//}

//bool DbSqlite2Instance::registerAggregateFunction(const QVariant& handle, const QString& name, int argCount)
//{
//    sqlite* sqliteHnd = getHandle(handle);
//    if (!sqliteHnd)
//        return false;

//    FunctionUserData* userData = new FunctionUserData;
//    userData->db = this;
//    userData->name = name;
//    userData->argCount = argCount;

//    int res = sqlite_create_aggregate(sqliteHnd, name.toLatin1().data(), argCount,
//                                      &DbSqlite2Instance::evaluateAggregateStep,
//                                      &DbSqlite2Instance::evaluateAggregateFinal,
//                                      userData);

//    return res == SQLITE_OK;
//}

//void DbSqlite2Instance::storeResult(sqlite_func* func, const QVariant& result, bool ok)
//{
//    if (!ok)
//    {
//        QByteArray ba = result.toString().toUtf8();
//        sqlite_set_result_error(func, ba.constData(), ba.size());
//        return;
//    }

//    // Code below is a modified code from Qt (its SQLite plugin).
//    if (result.isNull())
//    {
//        sqlite_set_result_string(func, nullptr, -1);
//        return;
//    }

//    switch (result.type())
//    {
//        case QVariant::ByteArray:
//        {
//            QByteArray ba = result.toByteArray();
//            sqlite_set_result_string(func, ba.constData(), ba.size());
//            break;
//        }
//        case QVariant::Int:
//        case QVariant::Bool:
//        case QVariant::UInt:
//        case QVariant::LongLong:
//        {
//            sqlite_set_result_int(func, result.toInt());
//            break;
//        }
//        case QVariant::Double:
//        {
//            sqlite_set_result_double(func, result.toDouble());
//            break;
//        }
//        default:
//        {
//            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
//            QByteArray ba = result.toString().toUtf8();
//            sqlite_set_result_string(func, ba.constData(), ba.size());
//            break;
//        }
//    }
//}

//QList<QVariant> DbSqlite2Instance::getArgs(int argCount, const char** args)
//{
//    QList<QVariant> results;

//    for (int i = 0; i < argCount; i++)
//    {
//        if (!args[i])
//        {
//            results << QVariant();
//            continue;
//        }

//        results << QString::fromUtf8(args[i]);
//    }
//    return results;
//}

//void DbSqlite2Instance::evaluateScalar(sqlite_func* func, int argCount, const char** args)
//{
//    void* dataPtr = sqlite_user_data(func);
//    if (!dataPtr)
//        return;

//    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
//    QList<QVariant> argList = getArgs(argCount, args);

//    bool ok;
//    QVariant result = FUNCTIONS->evaluateScalar(userData->name, userData->argCount, argList, userData->db, ok);

//    storeResult(func, result, ok);
//}

//void DbSqlite2Instance::evaluateAggregateStep(sqlite_func* func, int argCount, const char** args)
//{
//    void* dataPtr = sqlite_user_data(func);
//    if (!dataPtr)
//        return;

//    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
//    QList<QVariant> argList = getArgs(argCount, args);

//    QHash<QString,QVariant> aggregateContext = getAggregateContext(func);
//    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();
//    if (!aggregateContext.contains("initExecuted"))
//    {
//        FUNCTIONS->evaluateAggregateInitial(userData->name, userData->argCount, userData->db, storage);
//        aggregateContext["initExecuted"] = true;
//    }

//    FUNCTIONS->evaluateAggregateStep(userData->name, userData->argCount, argList, userData->db, storage);
//    aggregateContext["storage"] = storage;
//    setAggregateContext(func, aggregateContext);
//}

//void DbSqlite2Instance::evaluateAggregateFinal(sqlite_func* func)
//{
//    void* dataPtr = sqlite_user_data(func);
//    if (!dataPtr)
//        return;

//    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
//    QHash<QString,QVariant> aggregateContext = getAggregateContext(func);
//    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();

//    bool ok;
//    QVariant result = FUNCTIONS->evaluateAggregateFinal(userData->name, userData->argCount, userData->db, ok, storage);

//    storeResult(func, result, ok);

//    releaseAggregateContext(func);
//}

//void DbSqlite2Instance::deleteUserData(FunctionUserData* userData)
//{
//    if (!userData)
//        return;

//    delete userData;
//}

//QHash<QString, QVariant> DbSqlite2Instance::getAggregateContext(sqlite_func* func)
//{
//    void* memPtr = sqlite_aggregate_context(func, sizeof(QHash<QString,QVariant>**));
//    if (!memPtr)
//    {
//        qCritical() << "Could not allocate aggregate context.";
//        return QHash<QString, QVariant>();
//    }

//    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
//    if (!*aggCtxPtr)
//        *aggCtxPtr = new QHash<QString,QVariant>();

//    return **aggCtxPtr;
//}

//void DbSqlite2Instance::setAggregateContext(sqlite_func* func, const QHash<QString, QVariant>& aggregateContext)
//{
//    void* memPtr = sqlite_aggregate_context(func, sizeof(QHash<QString,QVariant>**));
//    if (!memPtr)
//    {
//        qCritical() << "Could not extract aggregate context.";
//        return;
//    }

//    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
//    **aggCtxPtr = aggregateContext;
//}

//void DbSqlite2Instance::releaseAggregateContext(sqlite_func* func)
//{
//    void* memPtr = sqlite_aggregate_context(func, sizeof(QHash<QString,QVariant>**));
//    if (!memPtr)
//    {
//        qCritical() << "Could not release aggregate context.";
//        return;
//    }

//    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
//    delete *aggCtxPtr;
//}

//sqlite* DbSqlite2Instance::getHandle(const QVariant& handle)
//{
//    if (qstrcmp(handle.typeName(), "sqlite*") != 0)
//    {
//        qWarning() << "Direct function call on DbSqlite2Instance object, but driver handle is not sqlite*, its:" << handle.typeName();
//        return nullptr;
//    }
//    return *static_cast<sqlite**>(const_cast<void*>(handle.data()));
//}
