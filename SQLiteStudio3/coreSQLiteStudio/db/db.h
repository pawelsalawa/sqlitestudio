#ifndef DB_H
#define DB_H

#include <QVariant>
#include "common/global.h"
#include "coreSQLiteStudio_global.h"
#include "db/attachguard.h"
#include "interruptable.h"
#include "dbobjecttype.h"
#include "common/column.h"
#include <QObject>
#include <QList>
#include <QHash>
#include <QReadWriteLock>
#include <QRunnable>
#include <QStringList>
#include <QSet>
#include <QDataStream>

/** @file */

class AsyncQueryRunner;
class Db;
class DbManager;
class SqlQuery;

typedef QSharedPointer<SqlQuery> SqlQueryPtr;

/**
 * @brief Option to make new Db instance not install any functions or collations in the database.
 *
 * This connection option should be used (with boolean value = true) when creating Db instance
 * to be used internally (not exposed to the user) and you don't want any special features
 * (like custom SQL functions, custom collations) to be registered in that database.
 */
static_char* DB_PURE_INIT = "sqlitestudio_pure_db_initalization";

/**
 * @brief Option name for plugin handling the database.
 *
 * This is a constant naming the connection option, which tells SQLiteStudio which plugin was dedicated to handle
 * particular database.
 */
static_char* DB_PLUGIN = "plugin";

/**
 * @brief Database managed by application.
 *
 * Everything you might want to do with SQLite databases goes through this interface in the application.
 * It's has a common interface for common database operations, such as connecting and disconnecting,
 * checking current status, executing queries and reading results.
 * It keeps information about the database version, encoding (UTF-8, UTF-16, etc.),
 * symbolic name of the database and path to the file.
 *
 * Regular routine with the database object would be to open it (if not open yet), execute some query
 * and collect results. It can be done in several ways, but here's simple one:
 * @code
 * QList<int> queryDb(const QString& dbName, const QString& colValue1, int colValue2)
 * {
 *     // Getting database object by its name and opening it if necessary
 *     Db* db = DBLIST->getDb(dbName);
 *     if (!db)
 *         return; // no such database
 *
 *     if (!db->isOpen())
 *         db->open();
 *
 *     // Executing query and getting results
 *     SqlQueryPtr results = db->exec("SELECT intCol FROM table WHERE col1 = ?, col2 = ?", colValue1, colValue2)
 *
 *     QList<int> resultList;
 *     SqlResultsRowPtr row;
 *     while (row = results->next())
 *     {
 *         resultList << row->value("intCol").toInt();
 *     }
 *     return resultList;
 * }
 * @endcode
 *
 * The example above is very generic way to do things. You can use many methods which simplifies tasks in case
 * you work with smaller data sets. For example:
 * @code
 * int getRowId(Db* db, int colVal)
 * {
 *     // We assume that db is already open
 *     return db->exec("SELECT rowid FROM table WHERE column = ?", colVal).getSingleCell().toInt();
 * }
 * @endcode
 *
 * To write some data into database you can write as this:
 * @code
 * void insert(Db* db, const QString& val1, int val2)
 * {
 *     // We assume that db is already open
 *     db->exec("INSERT INTO table (col1, col2) VALUES (?, ?)", val1, val2);
 * }
 * @endcode
 *
 * You can use named parameters:
 * @code
 * void insert(Db* db, const QString& val1, int val2)
 * {
 *     QHash<QString,QVariant> params;
 *     params["c1"] = val1;
 *     params["c2"] = val2;
 *     db->exec("INSERT INTO table (col1, col2) VALUES (:c1, :c2)", params);
 * }
 * @endcode
 *
 * To check if the execution was successful, test results:
 * @code
 * void insert(Db* db, const QString& val1, int val2)
 * {
 *     SqlQueryPtr results = db->exec("INSERT INTO table (col1, col2) VALUES (?, ?)", val1, val2);
 *     if (results->isError())
 *     {
 *         qWarning() << "Error while inserting:" << results->getErrorCode() << results->getErrorText();
 *     }
 * }
 * @endcode
 *
 * @see DbBase
 * @see DbQt
 * @see DbQt2
 * @see DbQt3
 */
class API_EXPORT Db : public QObject, public Interruptable
{
    Q_OBJECT

    public:
        /**
         * @brief Flags for query execution.
         *
         * Those flags are used by exec() and asyncExec(). They can be used with bit-wise operators.
         */
        enum class Flag
        {
            NONE                = 0x0, /**< No flags. This is default. */
            PRELOAD             = 0x1, /**< Preloads all execution results into the results object. Useful for asynchronous execution. */
            NO_LOCK             = 0x2, /**<
                                        * Prevents SQLiteStudio from setting the lock for execution on this base (not the SQLite lock,
                                        * just a Db internal lock for multi-threading access to the Db::exec()). This should be used
                                        * only in justified circumstances. That is when the Db call has to be done from within the part
                                        * of code, where the lock on Db was already set. Never (!) use this to ommit lock from different
                                        * threads. Justified situation is when you implement Db::initialDbSetup() in the derived class,
                                        * or when you implement SqlFunctionPlugin. Don't use it for the usual cases.
                                        */
            SKIP_DROP_DETECTION = 0x4, /**< Query execution will not notify about any detected objects dropped by the query.
                                        *   Benefit is that it speeds up execution. */
            SKIP_PARAM_COUNTING = 0x8, /**< During execution with arguments as list the number of bind parameters will not be verified.
                                        *   This speeds up execution at cost of possible error if bind params in query don't match number of args. */
        };
        Q_DECLARE_FLAGS(Flags, Flag)

        /**
         * @brief Function to handle SQL query results.
         *
         * The function has to accept single results object and return nothing.
         * After results are processed, they will be deleted automatically, no need to handle that.
         */
        typedef std::function<void(SqlQueryPtr)> QueryResultsHandler;

        /**
         * @brief Default, empty constructor.
         */
        Db();

        /**
         * @brief Releases resources.
         *
         * Detaches any attached databases and closes the database if open.
         */
        virtual ~Db();

        /**
         * @brief Registers Db in Qt meta subsystem.
         *
         * It's called at the application startup. Makes Db* supported by Qt meta subsystem.
         */
        static void metaInit();

        /**
         * @brief Converts flags into string representation.
         * @param flags Flags to convert. Can be multiple flags OR'ed.
         * @return Flags as string representation, for example: STRING_REPLACE_ARGS.
         */
        static QString flagsToString(Flags flags);

        /**
         * @brief Checks if database is open (connected).
         * @return true if the database is connected, or false otherwise.
         */
        virtual bool isOpen() = 0;

        /**
         * @brief Gets database symbolic name.
         * @return Database symbolic name (as it was defined in call to DbManager#addDb() or DbManager#updateDb()).
         */
        virtual QString getName() const = 0;

        /**
         * @brief Gets database file path.
         * @return Database file path (as it was defined in call to DbManager#addDb() or DbManager#updateDb()).
         */
        virtual QString getPath() const = 0;

        /**
         * @brief Gets SQLite version major number for this database.
         * @return Major version number, that is 3 for SQLite 3.x.x and nothing else for now.
         *
         * You don't have to open the database. This information is always available.
         */
        virtual quint8 getVersion() const = 0;

        /**
         * @brief Gets database encoding.
         * @return Database encoding as returned from SQLite query: <tt>PRAGMA encoding;</tt>
         *
         * If the database is not open, then this methods quickly opens it, queries the encoding and closes the database.
         * The opening and closing of the database is not visible outside, it's just an internal operation.
         */
        virtual QString getEncoding() = 0;

        /**
         * @brief Gets connection options.
         * @return Connection options, the same as were passed to DbManager#addDb() or DbManager#updateDb().
         */
        virtual QHash<QString,QVariant>& getConnectionOptions() = 0;

        /**
         * @brief Sets new name for the database.
         * @param value New name.
         *
         * This method works only on closed databases. If the database is open, then warning is logged
         * and function does nothing more.
         */
        virtual void setName(const QString& value) = 0;

        /**
         * @brief Sets new file path for the database.
         * @param value New file path.
         *
         * This method works only on closed databases. If the database is open, then warning is logged
         * and function does nothing more.
         */
        virtual void setPath(const QString& value) = 0;

        /**
         * @brief Sets connection options for the database.
         * @param value Connection options. See DbManager::addDb() for details.
         *
         * This method works only on closed databases. If the database is open, then warning is logged
         * and function does nothing more.
         */
        virtual void setConnectionOptions(const QHash<QString,QVariant>& value) = 0;

        /**
         * @brief Sets the timeout for waiting for the database to be unlocked.
         * @param secs Number of seconds.
         *
         * When the database is locked by another application, then the SQLiteStudio will wait given number
         * of seconds for the database to be released, before the execution error is reported.
         *
         * Set it to negative value to set infinite timeout.
         *
         * This doesn't involve locking done by SQLiteStudio internally (see Db::Flag::NO_LOCK), which doesn't time out.
         */
        virtual void setTimeout(int secs) = 0;

        /**
         * @brief Gets the current database lock waiting timeout value.
         * @return Number of seconds to wait for the database to be released.
         *
         * See setTimeout() for details.
         */
        virtual int getTimeout() const = 0;

        /**
         * @brief Provides information in expected result columns from given query.
         * @param query SQL query that returns results.
         * @return List of columns expected from the query if executed.
         *
         * This method used SQLite API functions, which leverage SQLite column metadata to identify
         * real column, table and database of the expected result columns. It also fills in column names as displayed
         * in the result set and declared type for certain column (if can be determined), whereas the actual type in each
         * result row may be different (as SQLite allows it explicitly).
         *
         * If the query is not the one that returns any results (i.e. not SELECT nor PRAGMA), then this command returns
         * empty list.
         */
        virtual QList<AliasedColumn> columnsForQuery(const QString& query) = 0;

        /**
         * @brief Executes SQL query.
         * @param query Query to be executed. Parameter placeholders can be either of: ?, :param, \@param, just don't mix different types in single query.
         * @param args List of values to bind to parameter placeholders. As those are unnamed parameters, the order is important.
         * @param flags Execution flags.
         * @return Execution results.
         *
         * Executes SQL query and returns results. If there was an error, the results will tell you when you call SqlResults::isError().
         *
         * Queries like SELECT, INSERT, UPDATE, and DELETE accept positional parameters, but only for column values. If you would like to pass table name
         * for SELECT, you would have to use Flags::STRING_REPLACE_ARGS and parameter placeholders in format %1, %2, %3, and so on. You cannot mix
         * string parameters (as for Flags::STRING_REPLACE_ARGS) and regular SQLite parameters in single query. If you really need to, then you should
         * build query string first (using QString::arg() for string parameters) and then pass it to exec(), which will accept SQLite parameters binding.
         *
         * If the query doesn't return any interesting results (for example it's INSERT) and you don't care about errors, you can safely ignore results object.
         * The result object is shared pointer, therefore it will delete itself if not used.
         *
         * Given C++11 you can initialize list with braces, like this:
         * @code
         * SqlQueryPtr results = db->exec("SELECT * FROM table WHERE c1 = ? AND c2 = ? AND c3 = ? AND c4 = ?",
         *                                      {45, 76, "test", 3.56});
         * @endcode
         */
        virtual SqlQueryPtr exec(const QString& query, const QList<QVariant> &args, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query using named parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: :param, \@param, just don't mix different types in single query.
         * @param args Map of parameter name and the value assigned to it.
         * @param flags Execution flags. See exec() for setails.
         * @return Execution results.
         *
         * Given C++11 you can initialize hash map with braces, like this:
         * @code
         * SqlQueryPtr results = db->exec("SELECT * FROM table WHERE id = :userId AND name = :firstName",
         *                                      {
         *                                          {":userId", 45},
         *                                          {":firstName", "John"}
         *                                      });
         * @endcode
         *
         * @overload
         */
        virtual SqlQueryPtr exec(const QString& query, const QHash<QString, QVariant>& args, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query.
         * @overload
         */
        virtual SqlQueryPtr exec(const QString &query, Db::Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query.
         * @overload
         */
        virtual SqlQueryPtr exec(const QString &query, const QVariant &arg) = 0;

        /**
         * @brief Executes SQL query.
         * @overload
         */
        virtual SqlQueryPtr exec(const QString &query, std::initializer_list<QVariant> argList) = 0;

        /**
         * @brief Executes SQL query.
         * @overload
         */
        virtual SqlQueryPtr exec(const QString &query, std::initializer_list<std::pair<QString,QVariant>> argMap) = 0;

        /**
         * @brief Executes SQL query asynchronously using list of parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: ?, :param, \@param, just don't mix different types in single query.
         * @param args List of parameter values to bind.
         * @param resultsHandler Function (can be lambda) to handle results. The function has to accept single SqlQueryPtr object and return nothing.
         * @param flags Execution flags. See exec() for setails.
         *
         * Asynchronous execution takes place in another thread. Once the execution is finished, the results handler function is called.
         *
         * Example:
         * @code
         * db->asyncExec("SELECT * FROM table WHERE col = ?", {5}, [=](SqlQueryPtr results)
         * {
         *     qDebug() << "Received" << results->rowCount() << "rows in results.";
         * });
         * @endcode
         */
        virtual void asyncExec(const QString& query, const QList<QVariant>& args, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query asynchronously using named parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: :param, \@param, just don't mix different types in single query.
         * @param args Map of parameter name and the value assigned to it.
         * @param resultsHandler Function (can be lambda) to handle results. The function has to accept single SqlQueryPtr object and return nothing.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload
         */
        virtual void asyncExec(const QString& query, const QHash<QString, QVariant>& args, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query asynchronously.
         * @param query Query to be executed. See exec() for details.
         * @param resultsHandler Function (can be lambda) to handle results. The function has to accept single SqlQueryPtr object and return nothing.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload
         */
        virtual void asyncExec(const QString& query, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query asynchronously using list of parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: ?, :param, \@param, just don't mix different types in single query.
         * @param args List of parameter values to bind.
         * @param flags Execution flags. See exec() for setails.
         * @return Asynchronous execution ID.
         *
         * Asynchronous execution takes place in another thread. Once the execution is finished, the results is provided
         * with asyncExecFinished() signal. You should get the ID from results of this method and compare it with ID
         * from the signal, so when it matches, it means that the results object from signal is the answer to this execution.
         *
         * It's recommended to use method version which takes function pointer for results handing, as it's more resiliant to errors in the code.
         *
         * Given C++11 you can initialize list with braces, like this:
         * @code
         * int asyncId = db->asyncExec("SELECT * FROM table WHERE c1 = ? AND c2 = ? AND c3 = ? AND c4 = ?",
         *                                      {45, 76, "test", 3.56});
         * @endcode
         */
        virtual quint32 asyncExec(const QString& query, const QList<QVariant>& args, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query asynchronously using named parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: :param, \@param, just don't mix different types in single query.
         * @param args Map of parameter name and the value assigned to it.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload
         *
         * It's recommended to use method version which takes function pointer for results handing, as it's more resiliant to errors in the code.
         */
        virtual quint32 asyncExec(const QString& query, const QHash<QString, QVariant>& args, Flags flags = Flag::NONE) = 0;

        /**
         * @brief Executes SQL query asynchronously.
         * @param query Query to be executed. See exec() for details.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload
         *
         * It's recommended to use method version which takes function pointer for results handing, as it's more resiliant to errors in the code.
         */
        virtual quint32 asyncExec(const QString& query, Flags flags = Flag::NONE) = 0;

        virtual SqlQueryPtr prepare(const QString& query) = 0;

        /**
         * @brief Begins SQL transaction.
         * @return true on success, or false on failure.
         *
         * This method uses basic "BEGIN" statement to begin transaction, therefore recurrent transactions are not supported.
         */
        virtual bool begin() = 0;

        /**
         * @brief Commits SQL transaction.
         * @return true on success, or false otherwise.
         */
        virtual bool commit() = 0;

        /**
         * @brief Rolls back the transaction.
         * @return true on success, or false otherwise (i.e. there was no transaction open, there was a connection problem, etc).
         */
        virtual bool rollback() = 0;

        /**
         * @brief Interrupts current execution asynchronously.
         *
         * It's almost the same as interrupt(), except it returns immediately, instead of waiting for the interruption to finish.
         * In case of some heavy queries the interruption process might take a little while.
         */
        virtual void asyncInterrupt() = 0;

        /**
         * @brief Checks if the database is readable at the moment.
         * @return true if the database is readable, or false otherwise.
         *
         * The database can be in 3 states: not locked, locked for reading or locked for writing.
         * If it's locked for writing, than it's not readable and this method will return false.
         * If it's locked for reading or not locked at all, then this method will return true.
         * Database can be locked by other threads executing their queries on the database.
         */
        virtual bool isReadable() = 0;

        /**
         * @brief Tells whether given SQL is a complete statement or not.
         * @return true if given SQL is complete SQL (or more), or it misses some part.
         */
        virtual bool isComplete(const QString& sql) const = 0;

        /**
         * @brief Checks if the database is writable at the moment.
         * @return true if the database is writable, or false otherwise.
         *
         * The database can be in 3 states: not locked, locked for reading or locked for writing.
         * If it's locked for writing (by other thread) or reading, than it's not writable and this method will return false.
         * If it's not locked at all, then this method will return true.
         * Database can be locked by other threads executing their queries on the database.
         */
        virtual bool isWritable() = 0;

        /**
         * @brief Tells if the database is valid for operating on it.
         * @return true if the databse is valid, false otherwise.
         *
         * A valid database is the one that has valid path and driver plugin support loaded.
         * Invalid database is the one that application failed to load. Those are marked with the exclamation icon on the UI.
         */
        virtual bool isValid() const = 0;

        /**
         * @brief Attaches given database to this database.
         * @param otherDb Other registered database object.
         * @param silent If true, no errors or warnings will be reported to the NotifyManager (they will still appear in logs).
         * @return Name of the attached database (it's not the symbolic name of the other database, it's a name you would use in <tt>ATTACH 'name'</tt> statement),
         * or null string if error occurred.
         *
         * This is convinent method to attach other registered databases to this database. It generates attached database name, so it doesn't conflict
         * with other - already attached - database names, attaches the database with that name and returns that name to you, so you can refer to it in queries.
         */
        virtual QString attach(Db* otherDb, bool silent = false) = 0;

        /**
         * @brief Attaches given database to this database using guarded attach.
         * @param otherDb Other registered database object.
         * @param silent If true, no errors or warnings will be reported to the NotifyManager (they will still appear in logs).
         * @return Guarded attach instance with the name of the attached database inside.
         *
         * The guarded attach automatically detaches attached database when the attach guard is destroyed (goes out of scope).
         * The AttachGuard is in fact a QSharedPointer, so you can pass it by value to other functions prolong attchment.
         */
        virtual AttachGuard guardedAttach(Db* otherDb, bool silent = false) = 0;

        /**
         * @brief Detaches given database from this database.
         * @param otherDb Other registered database object.
         *
         * If the otherDb is not attached, this method does nothing. Otherwise it calls <tt>DETACH</tt> statement using the attach name generated before by attach().
         * You don't have to provide the attach name, as Db class remembers those names internally.
         */
        virtual void detach(Db* otherDb) = 0;

        /**
         * @brief Detaches all attached databases.
         *
         * Detaches all attached databases. This includes only databases attached with attach(). Databases attached with manual <tt>ATTACH</tt> query execution
         * will not be detached.
         */
        virtual void detachAll() = 0;

        /**
         * @brief Gets attached databases.
         * @return Table of attached databases and the attach names used to attach them.
         *
         * This method returns only databases attached with attach() method.
         */
        virtual const QHash<Db*,QString>& getAttachedDatabases() = 0;

        /**
         * @brief Gets all attached databases.
         * @return Set of attach names.
         *
         * This method returns all attached database names (the attach names), including both those from attach() and manual <tt>ATTACH</tt> query execution.
         */
        virtual QSet<QString> getAllAttaches() = 0;

        /**
         * @brief Generates unique name for object to be created in the database.
         * @param attachedDbName Optional attach name, so the name will be in context of that database.
         * @return Unique object name.
         *
         * Queries database for all existing objects and then generates name that is not on that list.
         * The generated name is a random string of length 16.
         */
        virtual QString getUniqueNewObjectName(const QString& attachedDbName = QString()) = 0;

        /**
         * @brief Gets last error string from database driver.
         * @return Last encountered error.
         *
         * Result of this method is determinated by DbPlugin.
         */
        virtual QString getErrorText() = 0;

        /**
         * @brief Gets last error code from database driver.
         * @return Code of last encountered error.
         *
         * Result of this method is determinated by DbPlugin.
         */
        virtual int getErrorCode() = 0;

        /**
         * @brief Gets database type label.
         * @return Database type label.
         *
         * The database type label is used on UI to tell user what database it is (SQLite 3, SQLite 2, Encrypted SQLite 3, etc).
         * This is usually the same as DbPlugin::getTitle(), but getTitle() is used in list of plugins in configuration dialog,
         * while getTypeLabel() is used on databases list.
         */
        virtual QString getTypeLabel() const = 0;

        /**
         * @brief Gets C++ class name implementing this particular Db instance.
         * @return Class name.
         *
         * It can be used to distinguish between different drivers of Db instances. While getTypeLabel() can theoretically return
         * same labels for two different drivers, this method will always return distinct class name.
         */
        virtual QString getTypeClassName() const = 0;

        /**
         * @brief Initializes resources once the all derived Db classes are constructed.
         * @return true on success, false on failure.
         *
         * It's called just after this object was created. Implementation of this method can call virtual methods, which was a bad idea
         * to do in constructor (because of how it's works in C++, if you didn't know).
         *
         * It usually queries database for it's version, etc.
         */
        virtual bool initAfterCreated() = 0;

        /**
         * @brief Deregisters custom SQL function from this database.
         * @param name Name of the function.
         * @param argCount Number of arguments accepted by the function (-1 for undefined).
         * @return true if deregistering was successful, or false otherwise.
         *
         * @see FunctionManager
         */
        virtual bool deregisterFunction(const QString& name, int argCount) = 0;

        /**
         * @brief Registers scalar custom SQL function.
         * @param name Name of the function.
         * @param argCount Number of arguments accepted by the function (-1 for undefined).
         * @param deterministic The deterministic function flag used when registering the function.
         * @return true on success, false on failure.
         *
         * Scalar functions are evaluated for each row and their result is used in place of function invokation.
         * Example of SQLite built-in scalar function is abs(), or length().
         *
         * This method is used only to let the database know, that the given function exists in FunctionManager and we want it to be visible
         * in this database's context. When the function is called from SQL query, then the function execution is delegated to the FunctionManager.
         *
         * For details about usage of custom SQL functions see https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#custom-sql-functions
         *
         * @see FunctionManager
         */
        virtual bool registerScalarFunction(const QString& name, int argCount, bool deterministic) = 0;

        /**
         * @brief Registers aggregate custom SQL function.
         * @param name Name of the function.
         * @param argCount Number of arguments accepted by the function (-1 for undefined).
         * @param deterministic The deterministic function flag used when registering the function.
         * @return true on success, false on failure.
         *
         * Aggregate functions are used to aggregate many rows into single row. They are common in queries with GROUP BY statements.
         * The aggregate function in SQLite is actually implemented by 2 functions - one for executing per each row (and which doesn't return any result yet,
         * just collects the data) and then the second function, executed at the end. The latter one must return the result, which becomes the result
         * of aggregate function.
         *
         * Aggregate functions in SQLiteStudio are almost the same as in SQLite itself, except SQLiteStudio has also a third function, which is called
         * at the very begining, before the first "per step" function is called. It's used to initialize anything that the step function might need.
         *
         * This method is used only to let the database know, that the given function exists in FunctionManager and we want it to be visible
         * in this database's context. When the function is called from SQL query, then the function execution is delegated to the FunctionManager.
         *
         * For details about usage of custom SQL functions see https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#custom-sql-functions
         *
         * @see FunctionManager
         */
        virtual bool registerAggregateFunction(const QString& name, int argCount, bool deterministic) = 0;

        /**
         * @brief Registers a collation sequence implementation in the database.
         * @param name Name of the collation.
         * @return true on success, false on failure.
         *
         * Collations are not supported by SQLite 2, so this method will always fail for those databases.
         *
         * Collations are handled by CollationManager. Each collation managed by the manager has a code implemented to return -1, 0 or 1
         * when comparing 2 values in the database in order to sort query results. The name passed to this method is a name of the collation
         * as it is used in SQL queries and also the same name must be used when defining collation in Collations editor window.
         *
         * For details about usage of custom collations see https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#custom-sql-functions
         *
         * @see CollationManager
         */
        virtual bool registerCollation(const QString& name) = 0;

        /**
         * @brief Deregisters previously registered collation from this database.
         * @param name Collation name.
         * @return true on success, false on failure.
         *
         * See registerCollation() for details on custom collations.
         */
        virtual bool deregisterCollation(const QString& name) = 0;

        /**
         * @brief Loads a SQLite extension.
         * @param filePath Absolute path to the extension file (dll/so/dylib).
         * @param initFunc Optional entry point function. If empty, SQLite's default will be used.
         * @return true on success, or false on failure.
         *
         * This function works only on SQLite 3 drivers, as SQLite 2 does not support extensions.
         * More details can be found at https://sqlite.org/c3ref/load_extension.html
         *
         * If function returns false, use getErrorText() to discover details.
         */
        virtual bool loadExtension(const QString& filePath, const QString& initFunc = QString()) = 0;

        /**
         * @brief Creates instance of same (derived) class with same construction parameters passed.
         * @return Created instance.
         *
         * This is useful when one needs to operate on this database out of DbManager context,
         * so ownership, connection/disconnection, deletion, etc. all belongs to the caller of this method.
         */
        virtual Db* clone() const = 0;

    signals:
        /**
         * @brief Emitted when the connection to the database was established.
         */
        void connected();

        /**
         * @brief Emitted after connection to the database was closed.
         */
        void disconnected();

        /**
         * @brief Emitted when other database was attached to this datbase.
         * @param db Other database that was attached.
         *
         * This is emitted only when the database was attached with attach() call.
         * Manual "ATTACH" query execution doesn't cause this signal to be emitted.
         */
        void attached(Db* db);

        /**
         * @brief Emitted when other database was detached from this datbase.
         * @param db Other database that was detached.
         *
         * This is emitted only when the database was detached with detach() call.
         * Manual "DETACH" query execution doesn't cause this signal to be emitted.
         */
        void detached(Db* db);

        //void attached(QString db); // TODO emit when called by user's sql
        //void detached(QString db); // TODO emit when called by user's sql

        /**
         * @brief Emitted when the asynchronous execution was finished.
         * @param asyncId Asynchronous ID.
         * @param results Results from query execution.
         *
         * This signal is emitted only when no handler function was passed to asyncExec().
         * It's emitted, so the results can be handled.
         * Always test \p asyncId if it's equal to ID returned from asyncExec().
         */
        void asyncExecFinished(quint32 asyncId, SqlQueryPtr results);

        /**
         * @brief idle Database became idle and awaits for instructions.
         *
         * This signal is emited after async execution has finished.
         * It is important to re-check isWritable() or isReadable()
         * in any slot connected to this signal, because some other slot
         * called before currently processed slot could already order
         * another async execution.
         */
        void idle();

        /**
         * @brief Emitted when any database object (table, index, trigger, or view) was just deleted from this database.
         * @param database Database (attach) name from which the object was deleted. Usually the "main".
         * @param name Name of the object deleted.
         * @param type Type of the object deleted.
         *
         * This signal covers only deletions made by this database of course. Deletions made by any other application
         * are not announced by this signal (as this is impossible to detect it just like that).
         */
        void dbObjectDeleted(const QString& database, const QString& name, DbObjectType type);

        /**
         * @brief Emitted just before disconnecting and user can deny it.
         * @param disconnectingDenied If set to true by anybody, then disconnecting is aborted.
         */
        void aboutToDisconnect(bool& disconnectingDenied);

    public slots:
        /**
         * @brief Opens connection to the database.
         * @return true on success, false on error.
         *
         * Emits connected() only on success.
         */
        virtual bool open() = 0;

        /**
         * @brief Closes connection to the database.
         * @return true on success, false on error.
         *
         * Emits disconnected() only on success (i.e. db was open before).
         */
        virtual bool close() = 0;

        /**
         * @brief Opens connection to the database quietly.
         * @return true on success, false on error.
         *
         * Opens database, doesn't emit any signal.
         */
        virtual bool openQuiet() = 0;

        /**
         * @brief Opens connection to the database quietly, without applying any specific settings.
         * @return true on success, false on error.
         *
         * Opens database, doesn't emit any signal. It also doesn't apply any pragmas, neither registers
         * functions or collations. It should be used when you want to do some basic query on the database,
         * like when you probe the database for being the correct database for this implementation (driver, etc).
         * Actually, that's what DbPluginSqlite3 plugin (among others) use.
         *
         * To close database open with this method use closeQuiet().
         */
        virtual bool openForProbing() = 0;

        /**
         * @brief Closes connection to the database quietly.
         * @return true on success, false on error.
         *
         * Closes database, doesn't emit any signal.
         */
        virtual bool closeQuiet() = 0;

        /**
         * @brief Deregisters all functions registered in the database and registers new (possibly the same) functions.
         *
         * This slot is called from openAndSetup() and then every time user modifies custom SQL functions and commits changes to them.
         * It deregisters all functions registered before in this database and registers new functions, currently defined for
         * this database.
         *
         * @see FunctionManager
         */
        virtual void registerAllFunctions() = 0;

        /**
         * @brief Deregisters all collations registered in the database and registers new (possibly the same) collations.
         *
         * This slot is called from openAndsetup() and then every time user modifies custom collations and commits changes to them.
         */
        virtual void registerAllCollations() = 0;
};

QDataStream &operator<<(QDataStream &out, const Db* myObj);
QDataStream &operator>>(QDataStream &in, Db*& myObj);

QDebug operator<<(QDebug dbg, const Db* db);

Q_DECLARE_METATYPE(Db*)
Q_DECLARE_OPERATORS_FOR_FLAGS(Db::Flags)

#endif // DB_H
