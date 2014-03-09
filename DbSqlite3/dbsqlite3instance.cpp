#include "dbsqlite3instance.h"
#include "functionmanager.h"
#include <sqlite3.h>
#include <QDebug>

DbSqlite3Instance::DbSqlite3Instance(const QString &driverName, const QString &type)
    : DbQt3(driverName, type)
{
}

void DbSqlite3Instance::interruptExecutionOnHandle(const QVariant &handle)
{
    sqlite3* sqliteHnd = getHandle(handle);
    if (!sqliteHnd)
        return;

    sqlite3_interrupt(sqliteHnd);
}

bool DbSqlite3Instance::deregisterFunction(const QVariant& handle, const QString& name, int argCount)
{
    sqlite3* sqliteHnd = getHandle(handle);
    if (!sqliteHnd)
        return false;

    sqlite3_create_function(sqliteHnd, name.toLatin1().data(), argCount, SQLITE_UTF8, 0, nullptr, nullptr, nullptr);
    return true;
}

bool DbSqlite3Instance::registerScalarFunction(const QVariant& handle, const QString& name, int argCount)
{
    sqlite3* sqliteHnd = getHandle(handle);
    if (!sqliteHnd)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int res = sqlite3_create_function_v2(sqliteHnd, name.toLatin1().data(), argCount, SQLITE_UTF8, userData,
                                         &DbSqlite3Instance::evaluateScalar,
                                         nullptr,
                                         nullptr,
                                         &DbSqlite3Instance::deleteUserData);

    return res == SQLITE_OK;
}

bool DbSqlite3Instance::registerAggregateFunction(const QVariant& handle, const QString& name, int argCount)
{
    sqlite3* sqliteHnd = getHandle(handle);
    if (!sqliteHnd)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int res = sqlite3_create_function_v2(sqliteHnd, name.toLatin1().data(), argCount, SQLITE_UTF8, userData,
                                         nullptr,
                                         &DbSqlite3Instance::evaluateAggregateStep,
                                         &DbSqlite3Instance::evaluateAggregateFinal,
                                         &DbSqlite3Instance::deleteUserData);

    return res == SQLITE_OK;
}

void DbSqlite3Instance::initialDbSetup()
{
    exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
    exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
}

QList<QVariant> DbSqlite3Instance::getArgs(int argCount, sqlite3_value** args)
{
    int dataType;
    QList<QVariant> results;
    QVariant value;

    // The loop below uses slightly modified code from Qt (its SQLite plugin) to extract values.
    for (int i = 0; i < argCount; i++)
    {
        dataType = sqlite3_value_type(args[i]);
        switch (dataType)
        {
            case SQLITE_INTEGER:
                value = sqlite3_value_int64(args[i]);
                break;
            case SQLITE_BLOB:
                value = QByteArray(
                            static_cast<const char*>(sqlite3_value_blob(args[i])),
                            sqlite3_value_bytes(args[i])
                        );
                break;
            case SQLITE_FLOAT:
                value = sqlite3_value_double(args[i]);
                break;
            case SQLITE_NULL:
                value = QVariant(QVariant::String);
                break;
            default:
                value = QString(
                            reinterpret_cast<const QChar*>(sqlite3_value_text16(args[i])),
                            sqlite3_value_bytes16(args[i]) / sizeof(QChar)
                        );
                break;
        }
        results << value;
    }
    return results;
}

void DbSqlite3Instance::storeResult(sqlite3_context* context, const QVariant& result, bool ok)
{
    if (!ok)
    {
        QString str = result.toString();
        sqlite3_result_error16(context, str.utf16(), str.size() * sizeof(QChar));
        return;
    }

    // Code below is a modified code from Qt (its SQLite plugin).
    if (result.isNull())
    {
        sqlite3_result_null(context);
        return;
    }

    switch (result.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = result.toByteArray();
            sqlite3_result_blob(context, ba.constData(), ba.size(), SQLITE_TRANSIENT);
            break;
        }
        case QVariant::Int:
        case QVariant::Bool:
        {
            sqlite3_result_int(context, result.toInt());
            break;
        }
        case QVariant::Double:
        {
            sqlite3_result_double(context, result.toDouble());
            break;
        }
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            sqlite3_result_int64(context, result.toLongLong());
            break;
        }
        default:
        {
            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
            QString str = result.toString();
            sqlite3_result_text16(context, str.utf16(), str.size() * sizeof(QChar), SQLITE_TRANSIENT);
            break;
        }
    }
}

void DbSqlite3Instance::evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args)
{
    void* dataPtr = sqlite3_user_data(context);
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    QList<QVariant> argList = getArgs(argCount, args);

    bool ok;
    QVariant result = FUNCTIONS->evaluateScalar(userData->name, userData->argCount, argList, userData->db, ok);

    storeResult(context, result, ok);
}

void DbSqlite3Instance::evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args)
{
    void* dataPtr = sqlite3_user_data(context);
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    QList<QVariant> argList = getArgs(argCount, args);

    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);
    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();
    if (!aggregateContext.contains("initExecuted"))
    {
        FUNCTIONS->evaluateAggregateInitial(userData->name, userData->argCount, userData->db, storage);
        aggregateContext["initExecuted"] = true;
    }

    FUNCTIONS->evaluateAggregateStep(userData->name, userData->argCount, argList, userData->db, storage);
    aggregateContext["storage"] = storage;
    setAggregateContext(context, aggregateContext);
}

void DbSqlite3Instance::evaluateAggregateFinal(sqlite3_context* context)
{
    void* dataPtr = sqlite3_user_data(context);
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);
    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();

    bool ok;
    QVariant result = FUNCTIONS->evaluateAggregateFinal(userData->name, userData->argCount, userData->db, ok, storage);

    storeResult(context, result, ok);

    releaseAggregateContext(context);
}

void DbSqlite3Instance::deleteUserData(void* dataPtr)
{
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    delete userData;
}

QHash<QString, QVariant> DbSqlite3Instance::getAggregateContext(sqlite3_context* context)
{
    void* memPtr = sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
    if (!memPtr)
    {
        qCritical() << "Could not allocate aggregate context.";
        return QHash<QString, QVariant>();
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    if (!*aggCtxPtr)
        *aggCtxPtr = new QHash<QString,QVariant>();

    return **aggCtxPtr;
}

void DbSqlite3Instance::setAggregateContext(sqlite3_context* context, const QHash<QString, QVariant>& aggregateContext)
{
    void* memPtr = sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
    if (!memPtr)
    {
        qCritical() << "Could not extract aggregate context.";
        return;
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    **aggCtxPtr = aggregateContext;
}

void DbSqlite3Instance::releaseAggregateContext(sqlite3_context* context)
{
    void* memPtr = sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
    if (!memPtr)
    {
        qCritical() << "Could not release aggregate context.";
        return;
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    delete *aggCtxPtr;
}

sqlite3* DbSqlite3Instance::getHandle(const QVariant& handle)
{
    if (qstrcmp(handle.typeName(), "sqlite3*") != 0)
    {
        qWarning() << "Direct function call on DbSqlite3Instance object, but driver handle is not sqlite3*, its:" << handle.typeName();
        return nullptr;
    }

    return *static_cast<sqlite3**>(const_cast<void*>(handle.data()));
}
