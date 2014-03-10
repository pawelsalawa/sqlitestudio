#ifndef DBQT2_H
#define DBQT2_H

#include "coreSQLiteStudio_global.h"
#include "dbqt.h"
#include "parser/lexer.h"
#include <sqlite.h>

/**
 * @brief Variation of DbQt for SQLite 2.
 *
 * Inherit this when implementing Db for SQLite 2. In most cases you will only need
 * to create one public constructor, which forwards parameters to the DbQt3 constructor.
 * This be sufficient to implement SQLite 2 database plugin.
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
class DbQt2 : public DbQt
{
    public:
        /**
         * @brief Creates database object based on Qt database framework.
         * @param driverName Driver names as passed to QSqlDatabase::addDatabase().
         * @param type Database type (SQLite3, SQLite2 or other...) used as a database type presented to user.
         *
         * All values from this constructor are just passed to DbQt constructor.
         */
        DbQt2(const QString& driverName, const QString& type) : DbQt(driverName, type) {}

        /**
         * @brief Common internal execution routing for SQLite 2.
         * @param query Query to be executed.
         * @param args Arguments for query.
         * @return Execution results.
         *
         * This is a replacement method for the regular execInternal(). It adds named parameter placeholders support
         * for SQLite 2, which normally doesn't support them.
         */
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args)
        {
            // Sqlite2 seems to be dealing with only the '?' parameter placeholders. We need to update all of them.
            Lexer lexer(Dialect::Sqlite2);
            TokenList tokens = lexer.tokenize(query);
            TokenList filteredTokens = tokens.filter(Token::BIND_PARAM);
            foreach (TokenPtr token, filteredTokens)
                token->value = "?";

            QString newQuery = tokens.detokenize();
            return DbQt::execInternal(newQuery, args);
        }

        /**
         * @overload SqlResultsPtr execInternal(const QString &query, const QHash<QString, QVariant> &args)
         */
        SqlResultsPtr execInternal(const QString& query, const QHash<QString,QVariant>& args)
        {
            QList<QVariant> newArgs;

            Lexer lexer(Dialect::Sqlite2);
            TokenList tokens = lexer.tokenize(query);
            TokenList filteredTokens = tokens.filter(Token::BIND_PARAM);
            QString key;
            foreach (TokenPtr token, filteredTokens)
            {
                if (token->value.length() > 1)
                {
                    key = token->value.mid(1);
                    if (!args.contains(key))
                    {
                        qCritical() << "Named bind paramter cannot be found in hash (DbSqlite2Instance::execInternal()).";
                        newArgs << QVariant();
                    }
                    else
                        newArgs << args[key];
                }
                else
                {
                    qCritical() << "Unnamed bind paramter but execInternal() called with hash (DbSqlite2Instance::execInternal()).";
                    newArgs << QVariant();
                }

                token->value = "?";
            }

            QString newQuery = tokens.detokenize();
            return DbQt::execInternal(newQuery, newArgs);
        }

    protected:
        void interruptExecutionOnHandle(const QVariant& handle)
        {
            sqlite* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return;

            sqlite_interrupt(sqliteHnd);
        }

        bool deregisterFunction(const QVariant& handle, const QString& name, int argCount)
        {
            sqlite* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return false;

            sqlite_create_function(sqliteHnd, name.toLatin1().data(), argCount, nullptr, nullptr);
            sqlite_create_aggregate(sqliteHnd, name.toLatin1().data(), argCount, nullptr, nullptr, nullptr);

            FunctionUserData* userData = nullptr;
            QMutableListIterator<FunctionUserData*> it(userDataList);
            while (it.hasNext())
            {
                userData = it.next();
                if (userData->name == name && userData->argCount == argCount)
                {
                    it.remove();
                    delete userData;
                }
            }

            return true;
        }

        bool registerScalarFunction(const QVariant& handle, const QString& name, int argCount)
        {
            sqlite* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return false;

            FunctionUserData* userData = new FunctionUserData;
            userData->db = this;
            userData->name = name;
            userData->argCount = argCount;

            int res = sqlite_create_function(sqliteHnd, name.toLatin1().data(), argCount,
                                             &DbQt2<T>::evaluateScalar, userData);

            return res == SQLITE_OK;
        }

        bool registerAggregateFunction(const QVariant& handle, const QString& name, int argCount)
        {
            sqlite* sqliteHnd = getHandle(handle);
            if (!sqliteHnd)
                return false;

            FunctionUserData* userData = new FunctionUserData;
            userData->db = this;
            userData->name = name;
            userData->argCount = argCount;

            int res = sqlite_create_aggregate(sqliteHnd, name.toLatin1().data(), argCount,
                                              &DbQt2<T>::evaluateAggregateStep,
                                              &DbQt2<T>::evaluateAggregateFinal,
                                              userData);

            return res == SQLITE_OK;
        }

        static void storeResult(sqlite_func* func, const QVariant& result, bool ok)
        {
            if (!ok)
            {
                QByteArray ba = result.toString().toUtf8();
                sqlite_set_result_error(func, ba.constData(), ba.size());
                return;
            }

            // Code below is a modified code from Qt (its SQLite plugin).
            if (result.isNull())
            {
                sqlite_set_result_string(func, nullptr, -1);
                return;
            }

            switch (result.type())
            {
                case QVariant::ByteArray:
                {
                    QByteArray ba = result.toByteArray();
                    sqlite_set_result_string(func, ba.constData(), ba.size());
                    break;
                }
                case QVariant::Int:
                case QVariant::Bool:
                case QVariant::UInt:
                case QVariant::LongLong:
                {
                    sqlite_set_result_int(func, result.toInt());
                    break;
                }
                case QVariant::Double:
                {
                    sqlite_set_result_double(func, result.toDouble());
                    break;
                }
                default:
                {
                    // SQLITE_TRANSIENT makes sure that sqlite buffers the data
                    QByteArray ba = result.toString().toUtf8();
                    sqlite_set_result_string(func, ba.constData(), ba.size());
                    break;
                }
            }
        }

        static QList<QVariant> getArgs(int argCount, const char** args)
        {
            QList<QVariant> results;

            for (int i = 0; i < argCount; i++)
            {
                if (!args[i])
                {
                    results << QVariant();
                    continue;
                }

                results << QString::fromUtf8(args[i]);
            }
            return results;
        }

        static void evaluateScalar(sqlite_func* func, int argCount, const char** args)
        {
            QList<QVariant> argList = getArgs(argCount, args);
            bool ok = true;
            QVariant result = DbQt::evaluateScalar(sqlite_user_data(func), argList, ok);
            storeResult(func, result, ok);
        }

        static void evaluateAggregateStep(sqlite_func* func, int argCount, const char** args)
        {
            void* dataPtr = sqlite_user_data(func);
            QList<QVariant> argList = getArgs(argCount, args);
            QHash<QString,QVariant> aggregateContext = getAggregateContext(func);

            DbQt::evaluateAggregateStep(dataPtr, aggregateContext, argList);

            setAggregateContext(func, aggregateContext);
        }

        static void evaluateAggregateFinal(sqlite_func* func)
        {
            void* dataPtr = sqlite_user_data(func);
            QHash<QString,QVariant> aggregateContext = getAggregateContext(func);

            bool ok = true;
            QVariant result = DbQt::evaluateAggregateFinal(dataPtr, aggregateContext, ok);

            storeResult(func, result, ok);
            releaseAggregateContext(func);
        }

        static void* getContextMemPtr(sqlite_func* func)
        {
            return sqlite_aggregate_context(func, sizeof(QHash<QString,QVariant>**));
        }

        static QHash<QString,QVariant> getAggregateContext(sqlite_func* func)
        {
            return DbQt::getAggregateContext(getContextMemPtr(func));
        }

        static void setAggregateContext(sqlite_func* func, const QHash<QString,QVariant>& aggregateContext)
        {
            DbQt::setAggregateContext(getContextMemPtr(func), aggregateContext);
        }

        static void releaseAggregateContext(sqlite_func* func)
        {
            DbQt::releaseAggregateContext(getContextMemPtr(func));
        }

        sqlite* getHandle(const QVariant& handle)
        {
            if (qstrcmp(handle.typeName(), "sqlite*") != 0)
            {
                qWarning() << "Direct function call on DbSqlite2Instance object, but driver handle is not sqlite*, its:" << handle.typeName();
                return nullptr;
            }
            return *static_cast<sqlite**>(const_cast<void*>(handle.data()));
        }

        QList<FunctionUserData*> userDataList;
};

#endif // DBQT2_H
