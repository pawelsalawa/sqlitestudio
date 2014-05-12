#ifndef ABSTRACTDB3_H
#define ABSTRACTDB3_H

#include "db/abstractdb.h"
#include "parser/lexer.h"
#include "common/utils_sql.h"
#include "common/unused.h"
#include "services/collationmanager.h"
#include "sqlitestudio.h"
#include "db/sqlerrorcodes.h"
#include <sqlite3.h>
#include <QThread>
#include <QPointer>
#include <QDebug>

/**
 * @brief Complete implementation of SQLite 3 driver for SQLiteStudio.
 *
 * Inherit this when implementing Db for SQLite 3. In most cases you will only need
 * to create one public constructor, which forwards parameters to the AbstractDb constructor.
 * This be sufficient to implement SQLite 3 database plugin.
 * Just link it with proper SQLite library.
 *
 * The template parameter is currently not used for anything specific, so pass any unique type name.
 * The best would be to define empty class/structure just for this purpose.
 * The parameter is there, so this class becomes a template class.
 * We need a template class so we can provide common code base for all SQLite 3 plugins, while the
 * code isn't bound to the actual SQLite library. It's only bound to the specific library when the actual
 * linking is done.
 *
 * @see DbQt
 */
template <class T>
class AbstractDb3 : public AbstractDb
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb constructor.
         */
        AbstractDb3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);
        ~AbstractDb3();

    protected:
        bool isOpenInternal();
        void interruptExecution();
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        void initAfterOpen();
        SqlQueryPtr prepare(const QString& query);
        QString getTypeLabel();
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount);
        bool registerAggregateFunction(const QString& name, int argCount);
        bool registerCollationInternal(const QString& name);
        bool deregisterCollationInternal(const QString& name);

    private:
        class Query : public SqlQuery
        {
            public:
                class Row : public SqlResultsRow
                {
                    public:
                        int init(const QStringList& columns, sqlite3_stmt* stmt);

                    private:
                        int getValue(sqlite3_stmt* stmt, int col, QVariant& value);
                };

                Query(AbstractDb3<T>* db, const QString& query);
                ~Query();

                QString getErrorText();
                int getErrorCode();
                QStringList getColumnNames();
                int columnCount();
                qint64 rowsAffected();
                void finalize();

            protected:
                SqlResultsRowPtr nextInternal();
                bool hasNextInternal();
                bool execInternal(const QList<QVariant>& args);
                bool execInternal(const QHash<QString, QVariant>& args);

            private:
                int prepareStmt();
                int resetStmt();
                int bindParam(int paramIdx, const QVariant& value);
                int fetchFirst();
                int fetchNext();
                bool checkDbState();
                void copyErrorFromDb();
                void copyErrorToDb();
                void setError(int code, const QString& msg);

                QPointer<AbstractDb3<T>> db;
                sqlite3_stmt* stmt = nullptr;
                int errorCode = SQLITE_OK;
                QString errorMessage;
                int colCount = 0;
                QStringList colNames;
                int affected = 0;
                bool rowAvailable = false;
        };

        struct CollationUserData
        {
            QString name;
            AbstractDb3<T>* db;
        };

        QString extractLastError();
        void cleanUp();
        void resetError();

        /**
         * @brief Registers function to call when unknown collation was encountered by the SQLite.
         *
         * For unknown collations SQLite calls function registered by this method and expects it to register
         * proper function handling that collation, otherwise the query will result with error.
         *
         * The default collation handler does a simple QString::compare(), case insensitive.
         */
        void registerDefaultCollationRequestHandler();

        /**
         * @brief Stores given result in function's context.
         * @param context Custom SQL function call context.
         * @param result Value returned from function execution.
         * @param ok true if the result is from a successful execution, or false if the result contains error message (QString).
         *
         * This method is called after custom implementation of the function was evaluated and it returned the result.
         * It stores the result in function's context, so it becomes the result of the function call.
         */
        static void storeResult(sqlite3_context* context, const QVariant& result, bool ok);

        /**
         * @brief Converts SQLite arguments into the list of argument values.
         * @param argCount Number of arguments.
         * @param args SQLite argument values.
         * @return Convenient Qt list with argument values as QVariant.
         *
         * This function does necessary conversions reflecting internal SQLite datatype, so if the type
         * was for example BLOB, then the QVariant will be a QByteArray, etc.
         */
        static QList<QVariant> getArgs(int argCount, sqlite3_value** args);

        /**
         * @brief Evaluates requested function using defined implementation code and provides result.
         * @param context SQL function call context.
         * @param argCount Number of arguments passed to the function.
         * @param args Arguments passed to the function.
         *
         * This method is aware of the implementation language and the code defined for it,
         * so it delegates the execution to the proper plugin handling that language. Then it stores
         * result returned from the plugin in function's context, so it becomes function's result.
         *
         * This method is called for scalar functions.
         *
         * @see DbQt::evaluateScalar()
         */
        static void evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args);

        /**
         * @brief Evaluates requested function using defined implementation code and provides result.
         * @param context SQL function call context.
         * @param argCount Number of arguments passed to the function.
         * @param args Arguments passed to the function.
         *
         * This method is called for aggregate functions.
         *
         * If this is the first call to this function using this context, then it will execute
         * both "initial" and then "per step" code for this function implementation.
         *
         * @see DbQt3::evaluateScalar()
         * @see DbQt::evaluateAggregateStep()
         */
        static void evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args);

        /**
         * @brief Evaluates "final" code for aggregate function.
         * @param context SQL function call context.
         *
         * This method is called for aggregate functions.
         *
         * It's called once, at the end of aggregate function evaluation.
         * It executes "final" code of the function implementation.
         */
        static void evaluateAggregateFinal(sqlite3_context* context);

        /**
         * @brief Evaluates code of the collation.
         * @param userData Collation user data (name of the collation inside).
         * @param length1 Number of characters in value1 (excluding \0).
         * @param value1 First value to compare.
         * @param length2 Number of characters in value2 (excluding \0).
         * @param value2 Second value to compare.
         * @return -1, 0, or 1, as SQLite's collation specification demands it.
         */
        static int evaluateCollation(void* userData, int length1, const void* value1, int length2, const void* value2);

        /**
         * @brief Cleans up collation user data when collation is deregistered.
         * @param userData User data to delete.
         */
        static void deleteCollationUserData(void* userData);

        /**
         * @brief Destructor for function user data object.
         * @param dataPtr Pointer to the user data object.
         *
         * This is called by SQLite when the function is deregistered.
         */
        static void deleteUserData(void* dataPtr);

        /**
         * @brief Allocates and/or returns shared memory for the aggregate SQL function call.
         * @param context SQL function call context.
         * @return Pointer to the memory.
         *
         * It allocates exactly the number of bytes required to store pointer to a QHash.
         * The memory is released after the aggregate function is finished.
         */
        static void* getContextMemPtr(sqlite3_context* context);

        /**
         * @brief Allocates and/or returns QHash shared across all aggregate function steps.
         * @param context SQL function call context.
         * @return Shared hash table.
         *
         * The hash table is created before initial aggregate function step is made.
         * Then it's shared across all further steps (using this method to get it)
         * and then releases the memory after the last (final) step of the function call.
         */
        static QHash<QString,QVariant> getAggregateContext(sqlite3_context* context);

        /**
         * @brief Sets new value of the aggregate function shared hash table.
         * @param context SQL function call context.
         * @param aggregateContext New shared hash table value to store.
         *
         * This should be called after each time the context was requested with getAggregateContext() and then modified.
         */
        static void setAggregateContext(sqlite3_context* context, const QHash<QString,QVariant>& aggregateContext);

        /**
         * @brief Releases aggregate function shared hash table.
         * @param context SQL function call context.
         *
         * This should be called from final aggregate function step  to release the shared context (delete QHash).
         * The memory used to store pointer to the shared context will be released by the SQLite itself.
         */
        static void releaseAggregateContext(sqlite3_context* context);

        /**
         * @brief Registers default collation for requested collation.
         * @param fnUserData User data passed when registering collation request handling function.
         * @param fnDbHandle Database handle for which this call is being made.
         * @param eTextRep Text encoding (for now always SQLITE_UTF8).
         * @param collationName Name of requested collation.
         *
         * This function is called by SQLite to order registering collation with given name. We register default collation,
         * cause all known collations should already be registered.
         *
         * Default collation is implemented by evaluateDefaultCollation().
         */
        static void registerDefaultCollation(void* fnUserData, sqlite3* fnDbHandle, int eTextRep, const char* collationName);

        /**
         * @brief Called as a default collation implementation.
         * @param userData Collation user data, not used.
         * @param length1 Number of characters in value1 (excluding \0).
         * @param value1 First value to compare.
         * @param length2 Number of characters in value2 (excluding \0).
         * @param value2 Second value to compare.
         * @return -1, 0, or 1, as SQLite's collation specification demands it.
         */
        static int evaluateDefaultCollation(void* userData, int length1, const void* value1, int length2, const void* value2);

        sqlite3* dbHandle = nullptr;
        QString dbErrorMessage;
        int dbErrorCode = SQLITE_OK;
        QList<Query*> queries;

        /**
         * @brief User data for default collation request handling function.
         *
         * That function doesn't have destructor function pointer, so we need to keep track of that user data
         * and delete it when database is closed.
         */
        CollationUserData* defaultCollationUserData = nullptr;
};

//------------------------------------------------------------------------------------
// AbstractDb3
//------------------------------------------------------------------------------------

template <class T>
AbstractDb3<T>::AbstractDb3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb(name, path, connOptions)
{
}

template <class T>
AbstractDb3<T>::~AbstractDb3()
{
    if (isOpenInternal())
        closeInternal();
}

template <class T>
bool AbstractDb3<T>::isOpenInternal()
{
    return dbHandle != nullptr;
}

template <class T>
void AbstractDb3<T>::interruptExecution()
{
    if (!isOpenInternal())
        return;

    sqlite3_interrupt(dbHandle);
}

template <class T>
QString AbstractDb3<T>::getErrorTextInternal()
{
    return dbErrorMessage;
}

template <class T>
int AbstractDb3<T>::getErrorCodeInternal()
{
    return dbErrorCode;
}

template <class T>
bool AbstractDb3<T>::openInternal()
{
    resetError();
    sqlite3* handle;
    int res = sqlite3_open_v2(path.toUtf8().constData(), &handle, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, nullptr);
    if (!res == SQLITE_OK)
    {
        if (handle)
            sqlite3_close(handle);

        dbErrorMessage = tr("Could not open database: %1").arg(extractLastError());
        dbErrorCode = res;
        return false;
    }
    dbHandle = handle;
    return true;
}

template <class T>
bool AbstractDb3<T>::closeInternal()
{
    resetError();
    if (!dbHandle)
        return false;

    cleanUp();

    int res = sqlite3_close(dbHandle);
    if (res != SQLITE_OK)
    {
        dbErrorMessage = tr("Could not close database: %1").arg(extractLastError());
        dbErrorCode = res;
        qWarning() << "Error closing database. That's weird:" << dbErrorMessage;
        return false;
    }
    dbHandle = nullptr;
    return true;
}

template <class T>
void AbstractDb3<T>::initAfterOpen()
{
    sqlite3_enable_load_extension(dbHandle, true);
    registerDefaultCollationRequestHandler();;
    exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
    exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
}

template <class T>
SqlQueryPtr AbstractDb3<T>::prepare(const QString& query)
{
    return SqlQueryPtr(new Query(this, query));
}

template <class T>
QString AbstractDb3<T>::getTypeLabel()
{
    return T::label;
}

template <class T>
bool AbstractDb3<T>::deregisterFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    sqlite3_create_function(dbHandle, name.toUtf8().constData(), argCount, SQLITE_UTF8, 0, nullptr, nullptr, nullptr);
    return true;
}

template <class T>
bool AbstractDb3<T>::registerScalarFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int res = sqlite3_create_function_v2(dbHandle, name.toUtf8().constData(), argCount, SQLITE_UTF8, userData,
                                         &AbstractDb3<T>::evaluateScalar,
                                         nullptr,
                                         nullptr,
                                         &AbstractDb3<T>::deleteUserData);

    return res == SQLITE_OK;
}

template <class T>
bool AbstractDb3<T>::registerAggregateFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int res = sqlite3_create_function_v2(dbHandle, name.toUtf8().constData(), argCount, SQLITE_UTF8, userData,
                                         nullptr,
                                         &AbstractDb3<T>::evaluateAggregateStep,
                                         &AbstractDb3<T>::evaluateAggregateFinal,
                                         &AbstractDb3<T>::deleteUserData);

    return res == SQLITE_OK;
}

template <class T>
bool AbstractDb3<T>::registerCollationInternal(const QString& name)
{
    if (!dbHandle)
        return false;

    CollationUserData* userData = new CollationUserData;
    userData->name = name;

    int res = sqlite3_create_collation_v2(dbHandle, name.toUtf8().constData(), SQLITE_UTF8, userData,
                                          &AbstractDb3<T>::evaluateCollation,
                                          &AbstractDb3<T>::deleteCollationUserData);
    return res == SQLITE_OK;
}

template <class T>
bool AbstractDb3<T>::deregisterCollationInternal(const QString& name)
{
    if (!dbHandle)
        return false;

    sqlite3_create_collation_v2(dbHandle, name.toUtf8().constData(), SQLITE_UTF8, nullptr, nullptr, nullptr);
    return true;
}

template <class T>
QString AbstractDb3<T>::extractLastError()
{
    dbErrorCode = sqlite3_extended_errcode(dbHandle);
    dbErrorMessage = QString::fromUtf8(sqlite3_errmsg(dbHandle));
    return dbErrorMessage;
}

template <class T>
void AbstractDb3<T>::cleanUp()
{
    for (Query* q : queries)
        q->finalize();

    safe_delete(defaultCollationUserData);
}

template <class T>
void AbstractDb3<T>::resetError()
{
    dbErrorCode = 0;
    dbErrorMessage = QString::null;
}

template <class T>
void AbstractDb3<T>::storeResult(sqlite3_context* context, const QVariant& result, bool ok)
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

template <class T>
QList<QVariant> AbstractDb3<T>::getArgs(int argCount, sqlite3_value** args)
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

template <class T>
void AbstractDb3<T>::evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args)
{
    QList<QVariant> argList = getArgs(argCount, args);
    bool ok = true;
    QVariant result = AbstractDb::evaluateScalar(sqlite3_user_data(context), argList, ok);
    storeResult(context, result, ok);
}

template <class T>
void AbstractDb3<T>::evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args)
{
    void* dataPtr = sqlite3_user_data(context);
    QList<QVariant> argList = getArgs(argCount, args);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

    AbstractDb::evaluateAggregateStep(dataPtr, aggregateContext, argList);

    setAggregateContext(context, aggregateContext);
}

template <class T>
void AbstractDb3<T>::evaluateAggregateFinal(sqlite3_context* context)
{
    void* dataPtr = sqlite3_user_data(context);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

    bool ok = true;
    QVariant result = AbstractDb::evaluateAggregateFinal(dataPtr, aggregateContext, ok);

    storeResult(context, result, ok);
    releaseAggregateContext(context);
}

template <class T>
int AbstractDb3<T>::evaluateCollation(void* userData, int length1, const void* value1, int length2, const void* value2)
{
    UNUSED(length1);
    UNUSED(length2);
    CollationUserData* collUserData = reinterpret_cast<CollationUserData*>(userData);
    return COLLATIONS->evaluate(collUserData->name, QString::fromUtf8((const char*)value1), QString::fromUtf8((const char*)value2));
}

template <class T>
void AbstractDb3<T>::deleteCollationUserData(void* userData)
{
    if (!userData)
        return;

    CollationUserData* collUserData = reinterpret_cast<CollationUserData*>(userData);
    delete collUserData;
}

template <class T>
void AbstractDb3<T>::deleteUserData(void* dataPtr)
{
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    delete userData;
}

template <class T>
void* AbstractDb3<T>::getContextMemPtr(sqlite3_context* context)
{
    return sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
}

template <class T>
QHash<QString, QVariant> AbstractDb3<T>::getAggregateContext(sqlite3_context* context)
{
    return AbstractDb::getAggregateContext(getContextMemPtr(context));
}

template <class T>
void AbstractDb3<T>::setAggregateContext(sqlite3_context* context, const QHash<QString, QVariant>& aggregateContext)
{
    AbstractDb::setAggregateContext(getContextMemPtr(context), aggregateContext);
}

template <class T>
void AbstractDb3<T>::releaseAggregateContext(sqlite3_context* context)
{
    AbstractDb::releaseAggregateContext(getContextMemPtr(context));
}

template <class T>
void AbstractDb3<T>::registerDefaultCollation(void* fnUserData, sqlite3* fnDbHandle, int eTextRep, const char* collationName)
{
    CollationUserData* defUserData = reinterpret_cast<CollationUserData*>(fnUserData);
    if (!defUserData)
    {
        qWarning() << "Null userData in AbstractDb3<T>::registerDefaultCollation().";
        return;
    }

    AbstractDb3<T>* db = defUserData->db;
    if (!db)
    {
        qWarning() << "No database defined in userData of AbstractDb3<T>::registerDefaultCollation().";
        return;
    }

    // If SQLite seeks for collation implementation with different encoding, we force it to use existing one.
    if (db->isCollationRegistered(QString::fromUtf8(collationName)))
        return;

    // Check if dbHandle matches - just in case
    if (db->dbHandle != fnDbHandle)
    {
        qWarning() << "Mismatch of dbHandle in AbstractDb3<T>::registerDefaultCollation().";
        return;
    }

    int res = sqlite3_create_collation_v2(fnDbHandle, collationName, eTextRep, nullptr,
                                          &AbstractDb3<T>::evaluateDefaultCollation, nullptr);

    if (res != SQLITE_OK)
        qWarning() << "Could not register default collation in AbstractDb3<T>::registerDefaultCollation().";
}

template <class T>
int AbstractDb3<T>::evaluateDefaultCollation(void* userData, int length1, const void* value1, int length2, const void* value2)
{
    UNUSED(userData);
    UNUSED(length1);
    UNUSED(length2);
    return COLLATIONS->evaluateDefault(QString::fromUtf8((const char*)value1), QString::fromUtf8((const char*)value2));
}

template <class T>
void AbstractDb3<T>::registerDefaultCollationRequestHandler()
{
    if (!dbHandle)
        return;

    defaultCollationUserData = new CollationUserData;
    defaultCollationUserData->db = this;

    int res = sqlite3_collation_needed(dbHandle, defaultCollationUserData, &AbstractDb3<T>::registerDefaultCollation);
    if (res != SQLITE_OK)
        qWarning() << "Could not register default collation request handler. Unknown collations will cause errors.";
}

//------------------------------------------------------------------------------------
// Results
//------------------------------------------------------------------------------------

template <class T>
AbstractDb3<T>::Query::Query(AbstractDb3<T>* db, const QString& query) :
    db(db)
{
    this->query = query;
    db->queries << this;
}

template <class T>
AbstractDb3<T>::Query::~Query()
{
    if (db.isNull())
        return;

    finalize();
    db->queries.removeOne(this);
}

template <class T>
void AbstractDb3<T>::Query::copyErrorFromDb()
{
    if (db->dbErrorCode != 0)
    {
        errorCode = db->dbErrorCode;
        errorMessage = db->dbErrorMessage;
        return;
    }
}

template <class T>
void AbstractDb3<T>::Query::copyErrorToDb()
{
    db->dbErrorCode = errorCode;
    db->dbErrorMessage = errorMessage;
}

template <class T>
void AbstractDb3<T>::Query::setError(int code, const QString& msg)
{
    errorCode = code;
    errorMessage = msg;
    copyErrorToDb();
}

template <class T>
int AbstractDb3<T>::Query::prepareStmt()
{
    const char* tail;
    QByteArray queryBytes = query.toUtf8();
    int res = sqlite3_prepare_v2(db->dbHandle, queryBytes.constData(), queryBytes.size(), &stmt, &tail);
    if (res != SQLITE_OK)
    {
        stmt = nullptr;
        db->extractLastError();
        copyErrorFromDb();
        return res;
    }

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while executing query:" << query;

    return SQLITE_OK;
}

template <class T>
int AbstractDb3<T>::Query::resetStmt()
{
    errorCode = 0;
    errorMessage = QString::null;
    affected = 0;
    colCount = -1;
    rowAvailable = false;

    int res = sqlite3_reset(stmt);
    if (res != SQLITE_OK)
    {
        stmt = nullptr;
        setError(res, QString::fromUtf8(sqlite3_errmsg(db->dbHandle)));
        return res;
    }
    return SQLITE_OK;
}

template <class T>
bool AbstractDb3<T>::Query::execInternal(const QList<QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, Dialect::Sqlite3, flags.testFlag(Db::Flag::NO_LOCK));

    QueryWithParamCount queryWithParams = getQueryWithParamCount(query, Dialect::Sqlite3);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt();

    if (res != SQLITE_OK)
        return false;

    for (int paramIdx = 1; paramIdx <= queryWithParams.second; paramIdx++)
    {
        res = bindParam(paramIdx, args[paramIdx-1]);
        if (res != SQLITE_OK)
        {
            db->extractLastError();
            copyErrorFromDb();
            return false;
        }
    }

    bool ok = (fetchFirst() == SQLITE_OK);
    if (ok)
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
bool AbstractDb3<T>::Query::execInternal(const QHash<QString, QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, Dialect::Sqlite3, flags.testFlag(Db::Flag::NO_LOCK));

    QueryWithParamNames queryWithParams = getQueryWithParamNames(query, Dialect::Sqlite3);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt();

    if (res != SQLITE_OK)
        return false;

    int paramIdx = 1;
    foreach (const QString& paramName, queryWithParams.second)
    {
        if (!args.contains(paramName))
        {
            setError(SqlErrorCode::OTHER_EXECUTION_ERROR, "Error while preparing statement: could not bind parameter " + paramName);
            return false;
        }

        res = bindParam(paramIdx++, args[paramName]);
        if (res != SQLITE_OK)
        {
            db->extractLastError();
            copyErrorFromDb();
            return false;
        }
    }

    bool ok = (fetchFirst() == SQLITE_OK);
    if (ok)
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
int AbstractDb3<T>::Query::bindParam(int paramIdx, const QVariant& value)
{
    if (value.isNull())
    {
        return sqlite3_bind_null(stmt, paramIdx);
    }

    switch (value.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = value.toByteArray();
            return sqlite3_bind_blob(stmt, paramIdx, ba.constData(), ba.size(), SQLITE_TRANSIENT);
        }
        case QVariant::Int:
        case QVariant::Bool:
        {
            return sqlite3_bind_int(stmt, paramIdx, value.toInt());
        }
        case QVariant::Double:
        {
            return sqlite3_bind_double(stmt, paramIdx, value.toDouble());
        }
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            return sqlite3_bind_int64(stmt, paramIdx, value.toLongLong());
        }
        default:
        {
            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
            QString str = value.toString();
            return sqlite3_bind_text16(stmt, paramIdx, str.utf16(), str.size() * sizeof(QChar), SQLITE_TRANSIENT);
        }
    }

    return SQLITE_MISUSE; // not going to happen
}

template <class T>
bool AbstractDb3<T>::Query::checkDbState()
{
    if (db.isNull() || !db->dbHandle)
    {
        setError(SqlErrorCode::DB_NOT_DEFINED, "SqlQuery is no longer valid.");
        return false;
    }

    return true;
}

template <class T>
void AbstractDb3<T>::Query::finalize()
{
    if (stmt)
    {
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
}

template <class T>
QString AbstractDb3<T>::Query::getErrorText()
{
    return errorMessage;
}

template <class T>
int AbstractDb3<T>::Query::getErrorCode()
{
    return errorCode;
}

template <class T>
QStringList AbstractDb3<T>::Query::getColumnNames()
{
    return colNames;
}

template <class T>
int AbstractDb3<T>::Query::columnCount()
{
    return colCount;
}

template <class T>
qint64 AbstractDb3<T>::Query::rowsAffected()
{
    return affected;
}

template <class T>
SqlResultsRowPtr AbstractDb3<T>::Query::nextInternal()
{
    Row* row = new Row;
    int res = row->init(colNames, stmt);
    if (res != SQLITE_OK)
    {
        delete row;
        setError(res, QString::fromUtf8(sqlite3_errmsg(db->dbHandle)));
        return SqlResultsRowPtr();
    }

    fetchNext();
    return SqlResultsRowPtr(row);
}

template <class T>
bool AbstractDb3<T>::Query::hasNextInternal()
{
    return rowAvailable && stmt && checkDbState();
}

template <class T>
int AbstractDb3<T>::Query::fetchFirst()
{
    colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; i++)
        colNames << QString::fromUtf8(sqlite3_column_name(stmt, i));

    affected = sqlite3_changes(db->dbHandle);

    rowAvailable = true;
    int res = fetchNext();
    if (res == SQLITE_OK)
    {
        insertRowId["ROWID"] = sqlite3_last_insert_rowid(db->dbHandle);
    }

    return res;
}

template <class T>
int AbstractDb3<T>::Query::fetchNext()
{
    if (!checkDbState())
        rowAvailable = false;

    if (!rowAvailable || !stmt)
        return SQLITE_MISUSE;

    rowAvailable = false;
    int res;
    int secondsSpent = 0;
    while ((res = sqlite3_step(stmt)) == SQLITE_BUSY && secondsSpent < db->getTimeout())
    {
        QThread::sleep(1);
        if (db->getTimeout() >= 0)
            secondsSpent++;
    }

    switch (res)
    {
        case SQLITE_ROW:
            rowAvailable = true;
            break;
        case SQLITE_DONE:
            // Empty pointer as no more results are available.
            break;
        default:
            setError(res, QString::fromUtf8(sqlite3_errmsg(db->dbHandle)));
            return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

//------------------------------------------------------------------------------------
// Row
//------------------------------------------------------------------------------------

template <class T>
int AbstractDb3<T>::Query::Row::init(const QStringList& columns, sqlite3_stmt* stmt)
{
    int res = SQLITE_OK;
    QVariant value;
    for (int i = 0; i < columns.size(); i++)
    {
        res = getValue(stmt, i, value);
        if (res != SQLITE_OK)
            return res;

        values << value;
        valuesMap[columns[i]] = value;
    }
    return res;
}

template <class T>
int AbstractDb3<T>::Query::Row::getValue(sqlite3_stmt* stmt, int col, QVariant& value)
{
    int dataType = sqlite3_column_type(stmt, col);
    switch (dataType)
    {
        case SQLITE_INTEGER:
            value = sqlite3_column_int64(stmt, col);
            break;
        case SQLITE_BLOB:
            value = QByteArray(
                        static_cast<const char*>(sqlite3_column_blob(stmt, col)),
                        sqlite3_column_bytes(stmt, col)
                        );
            break;
        case SQLITE_FLOAT:
            value = sqlite3_column_double(stmt, col);
            break;
        case SQLITE_NULL:
            value = QVariant(QVariant::String);
            break;
        default:
            value = QString(
                            reinterpret_cast<const QChar*>(sqlite3_column_text16(stmt, col)),
                            sqlite3_column_bytes16(stmt, col) / sizeof(QChar)
                        );
            break;
    }
    return SQLITE_OK;
}

#endif // ABSTRACTDB3_H
