#ifndef DBQT3_H
#define DBQT3_H

#include "dbqt.h"
#include <sqlite3.h>

/**
 * @brief Complete implementation of SQLite 3 driver for SQLiteStudio.
 *
 * Inherit this when implementing Db for SQLite 3. In most cases you will only need
 * to create one public constructor, which forwards parameters to the DbQt3 constructor.
 * This be sufficient to implement SQLite 3 database plugin.
 * Just link it with proper SQLite library.
 *
 * The template parameter is currently not used for anything specific, so pass any unique type name.
 * The best would be to define empty class/structure just for this purpose.
 * The parameter is there, so this class becomes a template class.
 * We need a template class so we can provide common code base for all SQLite 2 plugins, while the
 * code doesn't introduce dependency to SQLite 2 library, until it's used, which is in SQLite 2 plugins.
 *
 * @see DbQt
 */
template <class T>
class DbQt3 : public DbQt
{
    public:
        /**
         * @brief Creates database object based on Qt database framework.
         * @param driverName Driver names as passed to QSqlDatabase::addDatabase().
         * @param type Database type (SQLite3, SQLite2 or other...) used as a database type presented to user.
         *
         * All values from this constructor are just passed to DbQt constructor.
         */
        DbQt3(const QString& driverName, const QString& type) : DbQt(driverName, type) {}

    protected:
        /**
         * @brief Interrupts query execution using Qt's database handle.
         * @param handle Database handle from QSqlDatabase.
         */
        void interruptExecutionOnHandle(const QVariant& handle)
        {
            sqlite3* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return;

            sqlite3_interrupt(sqliteHnd);
        }

        /**
         * @brief Deregisters custom SQL function from the database.
         * @param handle Database handle from QSqlDatabase.
         * @param name Function name.
         * @param argCount Declared number of arguments for the function.
         * @return true on success, false on failure.
         *
         * Deregistering method causes DbQt3::deleteUserData() to be called.
         */
        bool deregisterFunction(const QVariant& handle, const QString& name, int argCount)
        {
            sqlite3* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return false;

            sqlite3_create_function(sqliteHnd, name.toLatin1().data(), argCount, SQLITE_UTF8, 0, nullptr, nullptr, nullptr);
            return true;
        }

        /**
         * @brief Registers custom scalar SQL function in the database.
         * @param handle Database handle from QSqlDatabase.
         * @param name Function name.
         * @param argCount Declared number of arguments for the function.
         * @return true on success, false on failure.
         *
         * When the registered function is called from the query, then DbQt3::evaluateScalar() static method will be called.
         */
        bool registerScalarFunction(const QVariant& handle, const QString& name, int argCount)
        {
            sqlite3* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return false;

            FunctionUserData* userData = new FunctionUserData;
            userData->db = this;
            userData->name = name;
            userData->argCount = argCount;

            int res = sqlite3_create_function_v2(sqliteHnd, name.toLatin1().data(), argCount, SQLITE_UTF8, userData,
                                                 &DbQt3<T>::evaluateScalar,
                                                 nullptr,
                                                 nullptr,
                                                 &DbQt3<T>::deleteUserData);

            return res == SQLITE_OK;
        }

        /**
         * @brief Registers custom aggregate SQL function in the database.
         * @param handle Database handle from QSqlDatabase.
         * @param name Function name.
         * @param argCount Declared number of arguments for the function.
         * @return true on success, false on failure.
         *
         * When the registered function is called from the query, then DbQt3::evaluateAggregateStep()
         * and DbQt3::evaluateAggregateFinal() static methods will be called.
         */
        bool registerAggregateFunction(const QVariant& handle, const QString& name, int argCount)
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
                                                 &DbQt3<T>::evaluateAggregateStep,
                                                 &DbQt3<T>::evaluateAggregateFinal,
                                                 &DbQt3<T>::deleteUserData);

            return res == SQLITE_OK;
        }

        /**
         * @brief Initializes some pragma settings.
         *
         * Specificly, enables foreign keys and recursive triggers support.
         */
        void initialDbSetup()
        {
            exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
            exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
        }

        /**
         * @brief Stores given result in function's context.
         * @param context Custom SQL function call context.
         * @param result Value returned from function execution.
         * @param ok true if the result is from a successful execution, or false if the result contains error message (QString).
         *
         * This method is called after custom implementation of the function was evaluated and it returned the result.
         * It stores the result in function's context, so it becomes the result of the function call.
         */
        static void storeResult(sqlite3_context* context, const QVariant& result, bool ok)
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

        /**
         * @brief Converts SQLite arguments into the list of argument values.
         * @param argCount Number of arguments.
         * @param args SQLite argument values.
         * @return Convenient Qt list with argument values as QVariant.
         *
         * This function does necessary conversions reflecting internal SQLite datatype, so if the type
         * was for example BLOB, then the QVariant will be a QByteArray, etc.
         */
        static QList<QVariant> getArgs(int argCount, sqlite3_value** args)
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
        static void evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args)
        {
            QList<QVariant> argList = getArgs(argCount, args);
            bool ok;
            QVariant result = DbQt::evaluateScalar(sqlite3_user_data(context), argList, ok);
            storeResult(context, result, ok);
        }

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
        static void evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args)
        {
            void* dataPtr = sqlite3_user_data(context);
            QList<QVariant> argList = getArgs(argCount, args);
            QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

            DbQt::evaluateAggregateStep(dataPtr, aggregateContext, argList);

            setAggregateContext(context, aggregateContext);
        }

        /**
         * @brief Evaluates "final" code for aggregate function.
         * @param context SQL function call context.
         *
         * This method is called for aggregate functions.
         *
         * It's called once, at the end of aggregate function evaluation.
         * It executes "final" code of the function implementation.
         */
        static void evaluateAggregateFinal(sqlite3_context* context)
        {
            void* dataPtr = sqlite3_user_data(context);
            QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

            bool ok;
            QVariant result = DbQt::evaluateAggregateFinal(dataPtr, aggregateContext, ok);

            storeResult(context, result, ok);
            releaseAggregateContext(context);
        }

        /**
         * @brief Destructor for function user data object.
         * @param dataPtr Pointer to the user data object.
         *
         * This is called by SQLite when the function is deregistered.
         */
        static void deleteUserData(void* dataPtr)
        {
            if (!dataPtr)
                return;

            FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
            delete userData;
        }

        /**
         * @brief Allocates and/or returns shared memory for the aggregate SQL function call.
         * @param context SQL function call context.
         * @return Pointer to the memory.
         *
         * It allocates exactly the number of bytes required to store pointer to a QHash.
         * The memory is released after the aggregate function is finished.
         */
        static void* getContextMemPtr(sqlite3_context* context)
        {
            return sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
        }

        /**
         * @brief Allocates and/or returns QHash shared across all aggregate function steps.
         * @param context SQL function call context.
         * @return Shared hash table.
         *
         * The hash table is created before initial aggregate function step is made.
         * Then it's shared across all further steps (using this method to get it)
         * and then releases the memory after the last (final) step of the function call.
         */
        static QHash<QString,QVariant> getAggregateContext(sqlite3_context* context)
        {
            return DbQt::getAggregateContext(getContextMemPtr(context));
        }

        /**
         * @brief Sets new value of the aggregate function shared hash table.
         * @param context SQL function call context.
         * @param aggregateContext New shared hash table value to store.
         *
         * This should be called after each time the context was requested with getAggregateContext() and then modified.
         */
        static void setAggregateContext(sqlite3_context* context, const QHash<QString,QVariant>& aggregateContext)
        {
            DbQt::setAggregateContext(getContextMemPtr(context), aggregateContext);
        }

        /**
         * @brief Releases aggregate function shared hash table.
         * @param context SQL function call context.
         *
         * This should be called from final aggregate function step  to release the shared context (delete QHash).
         * The memory used to store pointer to the shared context will be released by the SQLite itself.
         */
        static void releaseAggregateContext(sqlite3_context* context)
        {
            DbQt::releaseAggregateContext(getContextMemPtr(context));
        }

        /**
         * @brief Extracts native SQLite 3 database handle from Qt's database handle.
         * @param handle Qt's database handle from QSqlDatabase.
         * @return Pointer to the native SQLite3 handle or null if the handle was invalid.
         */
        sqlite3* getHandle(const QVariant& handle)
        {
            if (qstrcmp(handle.typeName(), "sqlite3*") != 0)
            {
                qWarning() << "Direct function call on DbSqlite3Instance object, but driver handle is not sqlite3*, its:" << handle.typeName();
                return nullptr;
            }

            return *static_cast<sqlite3**>(const_cast<void*>(handle.data()));
        }
};

#endif // DBQT3_H
