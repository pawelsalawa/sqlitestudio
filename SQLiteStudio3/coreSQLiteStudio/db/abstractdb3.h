#ifndef ABSTRACTDB3_H
#define ABSTRACTDB3_H

#include "db/abstractdb.h"
#include "parser/lexer.h"
#include "common/utils_sql.h"
#include "common/unused.h"
#include "services/collationmanager.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
#include "db/sqlerrorcodes.h"
#include "log.h"
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
 * The template parameter should provide all necessary SQLite symbols used by this implementation.
 * This way every Db plugin can provide it's own symbols to work on SQLite and so it allows
 * for loading multiple SQLite libraries into the same application, while symbols in each library
 * can be different (and should be different, to avoid name conflicts and symbol overlapping).
 * See how it's done in dbsqlite3.h.
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

        bool loadExtension(const QString& filePath, const QString& initFunc = QString());
        bool isComplete(const QString& sql) const;
        QList<AliasedColumn> columnsForQuery(const QString& query);

    protected:
        bool isOpenInternal();
        void interruptExecution();
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        bool initAfterCreated();
        void initAfterOpen();
        SqlQueryPtr prepare(const QString& query);
        bool flushWalInternal();
        QString getTypeLabel() const;
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount, bool deterministic);
        bool registerAggregateFunction(const QString& name, int argCount, bool deterministic);
        bool registerCollationInternal(const QString& name);
        bool deregisterCollationInternal(const QString& name);
        bool isTransactionActive() const;

    private:
        class Query : public SqlQuery
        {
            public:
                class Row : public SqlResultsRow
                {
                    public:
                        int init(const QStringList& columns, typename T::stmt* stmt, Db::Flags flags);

                    private:
                        int getValue(typename T::stmt* stmt, int col, QVariant& value, Db::Flags flags);
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
                typename T::stmt* stmt = nullptr;
                int errorCode = T::OK;
                QString errorMessage;
                int colCount = 0;
                QStringList colNames;
                bool rowAvailable = false;
        };

        struct CollationUserData
        {
            QString name;
            AbstractDb3<T>* db = nullptr;
        };

        QString extractLastError();
        QString extractLastError(typename T::handle* handle);
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
        static void storeResult(typename T::context* context, const QVariant& result, bool ok);

        /**
         * @brief Converts SQLite arguments into the list of argument values.
         * @param argCount Number of arguments.
         * @param args SQLite argument values.
         * @return Convenient Qt list with argument values as QVariant.
         *
         * This function does necessary conversions reflecting internal SQLite datatype, so if the type
         * was for example BLOB, then the QVariant will be a QByteArray, etc.
         */
        static QList<QVariant> getArgs(int argCount, typename T::value** args);

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
        static void evaluateScalar(typename T::context* context, int argCount, typename T::value** args);

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
        static void evaluateAggregateStep(typename T::context* context, int argCount, typename T::value** args);

        /**
         * @brief Evaluates "final" code for aggregate function.
         * @param context SQL function call context.
         *
         * This method is called for aggregate functions.
         *
         * It's called once, at the end of aggregate function evaluation.
         * It executes "final" code of the function implementation.
         */
        static void evaluateAggregateFinal(typename T::context* context);

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
        static void* getContextMemPtr(typename T::context* context);

        /**
         * @brief Allocates and/or returns QHash shared across all aggregate function steps.
         * @param context SQL function call context.
         * @return Shared hash table.
         *
         * The hash table is created before initial aggregate function step is made.
         * Then it's shared across all further steps (using this method to get it)
         * and then releases the memory after the last (final) step of the function call.
         */
        static QHash<QString,QVariant> getAggregateContext(typename T::context* context);

        /**
         * @brief Sets new value of the aggregate function shared hash table.
         * @param context SQL function call context.
         * @param aggregateContext New shared hash table value to store.
         *
         * This should be called after each time the context was requested with getAggregateContext() and then modified.
         */
        static void setAggregateContext(typename T::context* context, const QHash<QString,QVariant>& aggregateContext);

        /**
         * @brief Releases aggregate function shared hash table.
         * @param context SQL function call context.
         *
         * This should be called from final aggregate function step  to release the shared context (delete QHash).
         * The memory used to store pointer to the shared context will be released by the SQLite itself.
         */
        static void releaseAggregateContext(typename T::context* context);

        /**
         * @brief Registers default collation for requested collation.
         * @param fnUserData User data passed when registering collation request handling function.
         * @param fnDbHandle Database handle for which this call is being made.
         * @param eTextRep Text encoding (for now always T::UTF8).
         * @param collationName Name of requested collation.
         *
         * This function is called by SQLite to order registering collation with given name. We register default collation,
         * cause all known collations should already be registered.
         *
         * Default collation is implemented by evaluateDefaultCollation().
         */
        static void registerDefaultCollation(void* fnUserData, typename T::handle* fnDbHandle, int eTextRep, const char* collationName);

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

        typename T::handle* dbHandle = nullptr;
        QString dbErrorMessage;
        int dbErrorCode = T::OK;
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

template<class T>
bool AbstractDb3<T>::loadExtension(const QString& filePath, const QString& initFunc)
{
    char* errMsg = nullptr;
    int res = T::load_extension(dbHandle, filePath.toUtf8().constData(), initFunc.isEmpty() ? nullptr : initFunc.toUtf8().constData(), &errMsg);
    if (res != T::OK)
    {
        dbErrorMessage = QObject::tr("Could not load extension %1: %2").arg(filePath, extractLastError());
        dbErrorCode = res;
        if (errMsg)
        {
            dbErrorMessage = QObject::tr("Could not load extension %1: %2").arg(filePath, QString::fromUtf8(errMsg));
            T::free(errMsg);
        }
        return false;
    }
    return true;
}

template<class T>
bool AbstractDb3<T>::isComplete(const QString& sql) const
{
    return T::complete(sql.toUtf8().constData());
}

template<class T>
QList<AliasedColumn> AbstractDb3<T>::columnsForQuery(const QString& query)
{
    QList<AliasedColumn> result;
    const char* tail;
    QByteArray queryBytes = query.toUtf8();
    typename T::stmt* stmt = nullptr;
    int res = T::prepare_v2(dbHandle, queryBytes.constData(), queryBytes.size(), &stmt, &tail);
    if (res != T::OK)
    {
        stmt = nullptr;
        extractLastError();
        T::finalize(stmt);
        return result;
    }

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty() && !removeComments(QString::fromUtf8(tail)).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while finding columns for query:" << query;


    int colCount = T::column_count(stmt);
    for (int i = 0; i < colCount; i++)
    {
        AliasedColumn col;
        col.setDatabase(QString::fromUtf8(T::column_database_name(stmt, i)));
        col.setTable(QString::fromUtf8(T::column_table_name(stmt, i)));
        col.setColumn(QString::fromUtf8(T::column_origin_name(stmt, i)));
        col.setAlias(QString::fromUtf8(T::column_name(stmt, i)));
        result << col;
    }

    T::finalize(stmt);
    return result;
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

    T::interrupt(dbHandle);
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
bool AbstractDb3<T>::flushWalInternal()
{
    resetError();
    if (!dbHandle)
        return false;

    int res = T::wal_checkpoint_v2(dbHandle, nullptr, T::CHECKPOINT_FULL, nullptr, nullptr);
    if (res != T::OK)
    {
        dbErrorMessage = QObject::tr("Could not run WAL checkpoint: %1").arg(extractLastError());
        dbErrorCode = res;
    }
    return res == T::OK;
}

template <class T>
bool AbstractDb3<T>::openInternal()
{
    resetError();
    typename T::handle* handle = nullptr;
    int res = T::open_v2(path.toUtf8().constData(), &handle, T::OPEN_READWRITE|T::OPEN_CREATE, nullptr);
    if (res != T::OK)
    {
        dbErrorMessage = QObject::tr("Could not open database: %1").arg(extractLastError(handle));
        dbErrorCode = res;
        if (handle)
            T::close(handle);

        return false;
    }
    dbHandle = handle;
    T::enable_load_extension(dbHandle, 1);
    return true;
}

template <class T>
bool AbstractDb3<T>::closeInternal()
{
    resetError();
    if (!dbHandle)
        return false;

    cleanUp();

    int res = T::close(dbHandle);
    if (res != T::OK)
    {
        dbErrorMessage = QObject::tr("Could not close database: %1").arg(extractLastError());
        dbErrorCode = res;
        qWarning() << "Error closing database. That's weird:" << dbErrorMessage;
        return false;
    }
    dbHandle = nullptr;
    return true;
}

template <class T>
bool AbstractDb3<T>::initAfterCreated()
{
    version = 3;
    return AbstractDb::initAfterCreated();
}

template <class T>
void AbstractDb3<T>::initAfterOpen()
{
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
QString AbstractDb3<T>::getTypeLabel() const
{
    return T::label;
}

template <class T>
bool AbstractDb3<T>::deregisterFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    T::create_function(dbHandle, name.toUtf8().constData(), argCount, T::UTF8, 0, nullptr, nullptr, nullptr);
    return true;
}

template <class T>
bool AbstractDb3<T>::registerScalarFunction(const QString& name, int argCount, bool deterministic)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int opts = T::UTF8;
    if (deterministic)
        opts |= T::DETERMINISTIC;

    int res = T::create_function_v2(dbHandle, name.toUtf8().constData(), argCount, opts, userData,
                                         &AbstractDb3<T>::evaluateScalar,
                                         nullptr,
                                         nullptr,
                                         &AbstractDb3<T>::deleteUserData);

    return res == T::OK;
}

template <class T>
bool AbstractDb3<T>::registerAggregateFunction(const QString& name, int argCount, bool deterministic)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int opts = T::UTF8;
    if (deterministic)
        opts |= T::DETERMINISTIC;

    int res = T::create_function_v2(dbHandle, name.toUtf8().constData(), argCount, opts, userData,
                                         nullptr,
                                         &AbstractDb3<T>::evaluateAggregateStep,
                                         &AbstractDb3<T>::evaluateAggregateFinal,
                                         &AbstractDb3<T>::deleteUserData);

    return res == T::OK;
}

template <class T>
bool AbstractDb3<T>::registerCollationInternal(const QString& name)
{
    if (!dbHandle)
        return false;

    CollationManager::CollationPtr collation = COLLATIONS->getCollation(name);
    if (collation == nullptr)
        return false;
    if (collation->type == CollationManager::CollationType::EXTENSION_BASED)
        return !(exec(collation->code, Flag::NO_LOCK)->isError());

    CollationUserData* userData = new CollationUserData;
    userData->name = name;

    int res = T::create_collation_v2(dbHandle, name.toUtf8().constData(), T::UTF8, userData,
                                          &AbstractDb3<T>::evaluateCollation,
                                          &AbstractDb3<T>::deleteCollationUserData);
    return res == T::OK;
}

template <class T>
bool AbstractDb3<T>::deregisterCollationInternal(const QString& name)
{
    if (!dbHandle)
        return false;

    T::create_collation_v2(dbHandle, name.toUtf8().constData(), T::UTF8, nullptr, nullptr, nullptr);
    return true;
}

template <class T>
QString AbstractDb3<T>::extractLastError()
{
    return extractLastError(dbHandle);
}

template<class T>
QString AbstractDb3<T>::extractLastError(typename T::handle* handle)
{
    dbErrorCode = T::extended_errcode(handle);
    dbErrorMessage = QString::fromUtf8(T::errmsg(handle));
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
    dbErrorMessage = QString();
}

template <class T>
void AbstractDb3<T>::storeResult(typename T::context* context, const QVariant& result, bool ok)
{
    if (!ok)
    {
        QString str = result.toString();
        T::result_error16(context, str.utf16(), str.size() * sizeof(QChar));
        return;
    }

    // Code below is a modified code from Qt (its SQLite plugin).
    if (result.isNull())
    {
        T::result_null(context);
        return;
    }

    switch (result.userType())
    {
        case QMetaType::QByteArray:
        {
            QByteArray ba = result.toByteArray();
            T::result_blob(context, ba.constData(), ba.size(), T::TRANSIENT());
            break;
        }
        case QMetaType::Int:
        case QMetaType::Bool:
        {
            T::result_int(context, result.toInt());
            break;
        }
        case QMetaType::Double:
        {
            T::result_double(context, result.toDouble());
            break;
        }
        case QMetaType::UInt:
        case QMetaType::LongLong:
        {
            T::result_int64(context, result.toLongLong());
            break;
        }
        case QMetaType::QVariantList:
        {
            QList<QVariant> list = result.toList();
            QStringList strList;
            for (const QVariant& v : list)
                strList << v.toString();

            QString str = strList.join(" ");
            T::result_text16(context, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
            break;
        }
        case QMetaType::QStringList:
        {
            QString str = result.toStringList().join(" ");
            T::result_text16(context, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
            break;
        }
        default:
        {
            // T::TRANSIENT makes sure that sqlite buffers the data
            QString str = result.toString();
            T::result_text16(context, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
            break;
        }
    }
}

template <class T>
QList<QVariant> AbstractDb3<T>::getArgs(int argCount, typename T::value** args)
{
    int dataType;
    QList<QVariant> results;
    QVariant value;

    // The loop below uses slightly modified code from Qt (its SQLite plugin) to extract values.
    for (int i = 0; i < argCount; i++)
    {
        dataType = T::value_type(args[i]);
        switch (dataType)
        {
            case T::INTEGER:
                value = T::value_int64(args[i]);
                break;
            case T::BLOB:
                value = QByteArray(
                            static_cast<const char*>(T::value_blob(args[i])),
                            T::value_bytes(args[i])
                            );
                break;
            case T::FLOAT:
                value = T::value_double(args[i]);
                break;
            case T::NULL_TYPE:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                value = QVariant(QVariant::String);
#else
                value = QVariant(QMetaType::fromType<QString>());
#endif
                break;
            default:
                value = QString(
                            reinterpret_cast<const QChar*>(T::value_text16(args[i])),
                            T::value_bytes16(args[i]) / sizeof(QChar)
                            );
                break;
        }
        results << value;
    }
    return results;
}

template <class T>
void AbstractDb3<T>::evaluateScalar(typename T::context* context, int argCount, typename T::value** args)
{
    QList<QVariant> argList = getArgs(argCount, args);
    bool ok = true;
    QVariant result = AbstractDb::evaluateScalar(T::user_data(context), argList, ok);
    storeResult(context, result, ok);
}

template <class T>
void AbstractDb3<T>::evaluateAggregateStep(typename T::context* context, int argCount, typename T::value** args)
{
    void* dataPtr = T::user_data(context);
    QList<QVariant> argList = getArgs(argCount, args);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

    AbstractDb::evaluateAggregateStep(dataPtr, aggregateContext, argList);

    setAggregateContext(context, aggregateContext);
}

template <class T>
void AbstractDb3<T>::evaluateAggregateFinal(typename T::context* context)
{
    void* dataPtr = T::user_data(context);
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
void* AbstractDb3<T>::getContextMemPtr(typename T::context* context)
{
    return T::aggregate_context(context, sizeof(QHash<QString,QVariant>**));
}

template <class T>
QHash<QString, QVariant> AbstractDb3<T>::getAggregateContext(typename T::context* context)
{
    return AbstractDb::getAggregateContext(getContextMemPtr(context));
}

template <class T>
void AbstractDb3<T>::setAggregateContext(typename T::context* context, const QHash<QString, QVariant>& aggregateContext)
{
    AbstractDb::setAggregateContext(getContextMemPtr(context), aggregateContext);
}

template <class T>
void AbstractDb3<T>::releaseAggregateContext(typename T::context* context)
{
    AbstractDb::releaseAggregateContext(getContextMemPtr(context));
}

template <class T>
void AbstractDb3<T>::registerDefaultCollation(void* fnUserData, typename T::handle* fnDbHandle, int eTextRep, const char* collationName)
{
    UNUSED(eTextRep);

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

    SqlQueryPtr results = db->exec("PRAGMA collation_list", Db::Flag::NO_LOCK|Db::Flag::SKIP_DROP_DETECTION);
    if (results->isError())
        qWarning() << "Unable to query existing collations while registering needed collation" << collationName << ":" << db->getErrorText();

    QStringList existingCollations = results->columnAsList<QString>("name");
    if (existingCollations.contains(collationName))
    {
        qDebug() << "Requested collation" << collationName << "already exists. Probably different input encoding was expected,"
                 << "but SQLite should deal with it. Skipping default collation registration.";
        return;
    }

    int res = T::create_collation_v2(fnDbHandle, collationName, T::UTF8, nullptr,
                                          &AbstractDb3<T>::evaluateDefaultCollation, nullptr);

    if (res != T::OK)
        qWarning() << "Could not register default collation in AbstractDb3<T>::registerDefaultCollation().";
    else
        notifyWarn(tr("Registered default collation on demand, under name: %1").arg(collationName));
}

template <class T>
int AbstractDb3<T>::evaluateDefaultCollation(void* userData, int length1, const void* value1, int length2, const void* value2)
{
    UNUSED(userData);
    UNUSED(length1);
    UNUSED(length2);
    return COLLATIONS->evaluateDefault(QString::fromUtf8((const char*)value1, length1), QString::fromUtf8((const char*)value2, length2));
}

template <class T>
void AbstractDb3<T>::registerDefaultCollationRequestHandler()
{
    if (!dbHandle)
        return;

    defaultCollationUserData = new CollationUserData;
    defaultCollationUserData->db = this;

    int res = T::collation_needed(dbHandle, defaultCollationUserData, &AbstractDb3<T>::registerDefaultCollation);
    if (res != T::OK)
        qWarning() << "Could not register default collation request handler. Unknown collations will cause errors.";
}

template <class T>
bool AbstractDb3<T>::isTransactionActive() const
{
    if (!dbHandle)
        return false;

    return !T::get_autocommit(dbHandle);
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
    if (errorCode != T::OK)
        return; // don't overwrite first error

    errorCode = code;
    errorMessage = msg;
    copyErrorToDb();
}

template <class T>
int AbstractDb3<T>::Query::prepareStmt()
{
    const char* tail;
    QByteArray queryBytes = query.toUtf8();
    int res = T::prepare_v2(db->dbHandle, queryBytes.constData(), queryBytes.size(), &stmt, &tail);
    if (res != T::OK)
    {
        stmt = nullptr;
        db->extractLastError();
        copyErrorFromDb();
        return res;
    }

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty() && !removeComments(QString::fromUtf8(tail)).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while executing query:" << query;

    return T::OK;
}

template <class T>
int AbstractDb3<T>::Query::resetStmt()
{
    errorCode = 0;
    errorMessage = QString();
    affected = 0;
    colCount = -1;
    rowAvailable = false;

    int res = T::reset(stmt);
    if (res != T::OK)
    {
        stmt = nullptr;
        setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
        return res;
    }
    return T::OK;
}

template <class T>
bool AbstractDb3<T>::Query::execInternal(const QList<QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, flags.testFlag(Db::Flag::NO_LOCK));
    logSql(db.data(), query, args, flags);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt();

    if (res != T::OK)
        return false;

    int maxParamIdx = args.size();
    if (!flags.testFlag(Db::Flag::SKIP_PARAM_COUNTING))
    {
        QueryWithParamCount queryWithParams = getQueryWithParamCount(query);
        maxParamIdx = qMin(maxParamIdx, queryWithParams.second);
    }

    for (int paramIdx = 1; paramIdx <= maxParamIdx; paramIdx++)
    {
        res = bindParam(paramIdx, args[paramIdx-1]);
        if (res != T::OK)
        {
            db->extractLastError();
            copyErrorFromDb();
            return false;
        }
    }

    bool ok = (fetchFirst() == T::OK);
    if (ok && !flags.testFlag(Db::Flag::SKIP_DROP_DETECTION))
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
bool AbstractDb3<T>::Query::execInternal(const QHash<QString, QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, flags.testFlag(Db::Flag::NO_LOCK));
    logSql(db.data(), query, args, flags);

    QueryWithParamNames queryWithParams = getQueryWithParamNames(query);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt();

    if (res != T::OK)
        return false;

    int paramIdx = -1;
    for (const QString& paramName : queryWithParams.second)
    {
        if (!args.contains(paramName))
        {
            qWarning() << "Could not bind parameter" << paramName << "because it was not found in passed arguments.";
            setError(SqlErrorCode::OTHER_EXECUTION_ERROR, "Error while preparing statement: could not bind parameter " + paramName);
            return false;
        }

        paramIdx = T::bind_parameter_index(stmt, paramName.toUtf8().constData());
        if (paramIdx <= 0)
        {
            qWarning() << "Could not bind parameter" << paramName << "because it was not found in prepared statement.";
            setError(SqlErrorCode::OTHER_EXECUTION_ERROR, "Error while preparing statement: could not bind parameter " + paramName);
            return false;
        }

        res = bindParam(paramIdx, args[paramName]);
        if (res != T::OK)
        {
            db->extractLastError();
            copyErrorFromDb();
            return false;
        }
    }

    bool ok = (fetchFirst() == T::OK);
    if (ok && !flags.testFlag(Db::Flag::SKIP_DROP_DETECTION))
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
int AbstractDb3<T>::Query::bindParam(int paramIdx, const QVariant& value)
{
    if (value.isNull())
    {
        return T::bind_null(stmt, paramIdx);
    }

    switch (value.userType())
    {
        case QMetaType::QByteArray:
        {
            QByteArray ba = value.toByteArray();
            return T::bind_blob(stmt, paramIdx, ba.constData(), ba.size(), T::TRANSIENT());
        }
        case QMetaType::Int:
        case QMetaType::Bool:
        {
            return T::bind_int(stmt, paramIdx, value.toInt());
        }
        case QMetaType::Double:
        {
            return T::bind_double(stmt, paramIdx, value.toDouble());
        }
        case QMetaType::UInt:
        case QMetaType::LongLong:
        {
            return T::bind_int64(stmt, paramIdx, value.toLongLong());
        }
        default:
        {
            // T::TRANSIENT makes sure that sqlite buffers the data
            QString str = value.toString();
            return T::bind_text16(stmt, paramIdx, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
        }
    }

    return T::MISUSE; // not going to happen
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
        T::finalize(stmt);
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
    int res = row->init(colNames, stmt, flags);
    if (res != T::OK)
    {
        delete row;
        setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
        return SqlResultsRowPtr();
    }

    res = fetchNext();
    if (res != T::OK)
    {
        delete row;
        return SqlResultsRowPtr();
    }

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
    colCount = T::column_count(stmt);
    for (int i = 0; i < colCount; i++)
        colNames << QString::fromUtf8(T::column_name(stmt, i));

    qint64 changesBefore =  T::total_changes64(db->dbHandle);
    rowAvailable = true;
    int res = fetchNext();

    affected = 0;
    if (res == T::OK)
    {
        affected =  T::total_changes64(db->dbHandle) - changesBefore;
        insertRowId["ROWID"] = T::last_insert_rowid(db->dbHandle);
    }

    return res;
}

template <class T>
int AbstractDb3<T>::Query::fetchNext()
{
    if (!checkDbState())
        rowAvailable = false;

    if (!rowAvailable || !stmt)
    {
        setError(T::MISUSE, QObject::tr("Result set expired or no row available."));
        return T::MISUSE;
    }

    rowAvailable = false;
    int res;
    int secondsSpent = 0;
    bool zeroTimeout = flags.testFlag(Db::Flag::ZERO_TIMEOUT);
    while ((res = T::step(stmt)) == T::BUSY && !zeroTimeout && secondsSpent < db->getTimeout() && !T::is_interrupted(db->dbHandle))
    {
        QThread::sleep(1);
        if (db->getTimeout() >= 0)
            secondsSpent++;
    }

    switch (res)
    {
        case T::ROW:
            rowAvailable = true;
            break;
        case T::DONE:
            // Empty pointer as no more results are available.
            break;
        case T::INTERRUPT:
            setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
            return T::INTERRUPT;
        default:
            setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
            return T::ERROR;
    }
    return T::OK;
}

//------------------------------------------------------------------------------------
// Row
//------------------------------------------------------------------------------------

template <class T>
int AbstractDb3<T>::Query::Row::init(const QStringList& columns, typename T::stmt* stmt, Db::Flags flags)
{
    int res = T::OK;
    QVariant value;
    for (int i = 0; i < columns.size(); i++)
    {
        res = getValue(stmt, i, value, flags);
        if (res != T::OK)
            return res;

        values << value;
        valuesMap[columns[i]] = value;
    }
    return res;
}

template <class T>
int AbstractDb3<T>::Query::Row::getValue(typename T::stmt* stmt, int col, QVariant& value, Db::Flags flags)
{
    UNUSED(flags);
    int dataType = T::column_type(stmt, col);
    switch (dataType)
    {
        case T::INTEGER:
            value = T::column_int64(stmt, col);
            break;
        case T::BLOB:
            value = QByteArray(
                        static_cast<const char*>(T::column_blob(stmt, col)),
                        T::column_bytes(stmt, col)
                        );
            break;
        case T::NULL_TYPE:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            value = QVariant(QVariant::String);
#else
            value = QVariant(QMetaType::fromType<QString>());
#endif
            break;
        case T::FLOAT:
            value = T::column_double(stmt, col);
            break;
        default:
            value = QString(
                            reinterpret_cast<const QChar*>(T::column_text16(stmt, col)),
                            T::column_bytes16(stmt, col) / sizeof(QChar)
                        );
            break;
    }
    return T::OK;
}

#endif // ABSTRACTDB3_H
