#ifndef DBQT3_H
#define DBQT3_H

#include "dbqt.h"
#include <sqlite3.h>

/**
 * @brief Complete implementation of SQLite 3 driver for SQLiteStudio.
 *
 * Inherit this when implementing Db for SQLite 3. In most cases you will only need
 * to create one public constructor, which forwards parameters to the DbQt3 constructor.
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
        void interruptExecutionOnHandle(const QVariant& handle)
        {
            sqlite3* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return;

            sqlite3_interrupt(sqliteHnd);
        }

        bool deregisterFunction(const QVariant& handle, const QString& name, int argCount)
        {
            sqlite3* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return false;

            sqlite3_create_function(sqliteHnd, name.toLatin1().data(), argCount, SQLITE_UTF8, 0, nullptr, nullptr, nullptr);
            return true;
        }

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

        void initialDbSetup()
        {
            exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
            exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
        }

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

        static void evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args)
        {
            QList<QVariant> argList = getArgs(argCount, args);
            bool ok;
            QVariant result = DbQt::evaluateScalar(sqlite3_user_data(context), argList, ok);
            storeResult(context, result, ok);
        }

        static void evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args)
        {
            void* dataPtr = sqlite3_user_data(context);
            QList<QVariant> argList = getArgs(argCount, args);
            QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

            DbQt::evaluateAggregateStep(dataPtr, aggregateContext, argList);

            setAggregateContext(context, aggregateContext);
        }

        static void evaluateAggregateFinal(sqlite3_context* context)
        {
            void* dataPtr = sqlite3_user_data(context);
            QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

            bool ok;
            QVariant result = DbQt::evaluateAggregateFinal(dataPtr, aggregateContext, ok);

            storeResult(context, result, ok);
            releaseAggregateContext(context);
        }

        static void deleteUserData(void* dataPtr)
        {
            if (!dataPtr)
                return;

            FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
            delete userData;
        }

        static void* getContextMemPtr(sqlite3_context* context)
        {
            return sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
        }

        static QHash<QString,QVariant> getAggregateContext(sqlite3_context* context)
        {
            return DbQt::getAggregateContext(getContextMemPtr(context));
        }

        static void setAggregateContext(sqlite3_context* context, const QHash<QString,QVariant>& aggregateContext)
        {
            DbQt::setAggregateContext(getContextMemPtr(context), aggregateContext);
        }

        static void releaseAggregateContext(sqlite3_context* context)
        {
            DbQt::releaseAggregateContext(getContextMemPtr(context));
        }

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
