#ifndef DB_H
#define DB_H

#include "../returncode.h"
#include "sqlresults.h"
#include "../dialect.h"
#include "readwritelocker.h"
#include "coreSQLiteStudio_global.h"
#include <QObject>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QReadWriteLock>
#include <QRunnable>
#include <QStringList>

/** @file */

class AsyncQueryRunner;
class Db;
class DbManager;

/**
 * @brief Database managed by application.
 * Everything you might want to do with SQLite databases goes through this class in the application.
 * It's has a common interface for common database operations, such as connecting and disconnecting,
 * checking current status, executing queries and reading results.
 * It keeps information about the database version, dialect (SQLite 2 vs SQLite 3), encoding (UTF-8, UTF-16, etc.),
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
 *     SqlResultsPtr results = db->exec("SELECT intCol FROM table WHERE col1 = ?, col2 = ?", colValue1, colValue2)
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
 *     SqlResultsPtr results = db->exec("INSERT INTO table (col1, col2) VALUES (?, ?)", val1, val2);
 *     if (results->isError())
 *     {
 *         qWarning() << "Error while inserting:" << results->getErrorCode() << results->getErrorText();
 *     }
 * }
 * @endcode
 *
 * \section interns Internals
 * If you implement derived class, you may want know the chain of method calls that is involved in executing the query.
 * It goes like this: // TODO put a diagram here
 */
class API_EXPORT Db : public QObject
{
    Q_OBJECT

    friend class DbManager;

    public:
        /**
         * @brief Flags for query execution.
         *
         * Those flags are used by exec() and asyncExec(). They can be used with bit-wise operators.
         */
        enum class Flag
        {
            NONE                = 0x0, /**< No flags. This is default. */
            STRING_REPLACE_ARGS = 0x1, /**<
                                        * Arguments are substituted with string replacing mechanism, instead of binding with query.
                                        * It uses QString::arg(), so parameter placeholders need to be in format %1, %2, %3, and so on.
                                        */
            PRELOAD             = 0x2, /**< Preloads all execution results into the results object. Useful for asynchronous execution. */
            NO_LOCK             = 0x4  /**<
                                        * Prevents SQLiteStudio from setting the lock for execution on this base (not the SQLite lock,
                                        * just a Db internal lock for multi-threading access to the Db::exec()). This should be used
                                        * only in justified circumstances. That is when the Db call has to be done from within the part
                                        * of code, where the lock on Db was already set. Never (!) use this to ommit lock from different
                                        * threads. Justified situation is when you implement Db::initialDbSetup() in the derived class,
                                        * or when you implement SqlFunctionPlugin. Don't use it for the usual cases.
                                        */
        };
        Q_DECLARE_FLAGS(Flags, Flag)

        /**
         * @brief Function to handle SQL query results.
         * The function has to accept single results object and return nothing.
         * After results are processed, they will be deleted automatically, no need to handle that.
         */
        typedef std::function<void(SqlResultsPtr)> QueryResultsHandler;

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
        bool isOpen();

        /**
         * @brief Gets database symbolic name.
         * @return Database symbolic name (as it was defined in call to DbManager#addDb() or DbManager#updateDb()).
         */
        QString getName();

        /**
         * @brief Gets database file path.
         * @return Database file path (as it was defined in call to DbManager#addDb() or DbManager#updateDb()).
         */
        QString getPath();

        /**
         * @brief Gets SQLite version major number for this database.
         * @return Major version number, that is 3 for SQLite 3.x.x and 2 for SQLite 2.x.x.
         *
         * You don't have to open the database. This information is always available.
         */
        quint8 getVersion();

        /**
         * @brief Gets database dialect.
         * @return Database dialect, which is either Sqlite2 or Sqlite3.
         *
         * You don't have to open the database. This information is always available.
         */
        Dialect getDialect();

        /**
         * @brief Gets database encoding.
         * @return Database encoding as returned from SQLite query: <tt>PRAGMA encoding;</tt>
         *
         * If the database is not open, then this methods quickly opens it, queries the encoding and closes the database.
         * The opening and closing of the database is not visible outside, it's just an internal operation.
         */
        QString getEncoding();

        /**
         * @brief Gets connection options.
         * @return Connection options, the same as were passed to DbManager#addDb() or DbManager#updateDb().
         */
        QHash<QString,QVariant>& getConnectionOptions();

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
         * SqlResultsPtr results = db->exec("SELECT * FROM table WHERE c1 = ? AND c2 = ? AND c3 = ? AND c4 = ?",
         *                                      {45, 76, "test", 3.56});
         * @endcode
         */
        SqlResultsPtr exec(const QString& query, const QList<QVariant> &args,
                           Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query using named parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: :param, \@param, just don't mix different types in single query.
         * @param args Map of parameter name and the value assigned to it.
         * @param flags Execution flags. See exec() for setails.
         * @return Execution results.
         *
         * Given C++11 you can initialize hash map with braces, like this:
         * @code
         * SqlResultsPtr results = db->exec("SELECT * FROM table WHERE id = :userId AND name = :firstName",
         *                                      {
         *                                          {":userId", 45},
         *                                          {":firstName", "John"}
         *                                      });
         * @endcode
         *
         * @overload SqlResultsPtr exec(const QString& query, const QHash<QString, QVariant>& args, Flags flags)
         */
        SqlResultsPtr exec(const QString& query, const QHash<QString, QVariant>& args, Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query.
         * @overload SqlResultsPtr exec(const QString &query, Db::Flags flags)
         */
        SqlResultsPtr exec(const QString &query, Db::Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query.
         * @overload SqlResultsPtr exec(const QString &query, const QVariant &value)
         */
        SqlResultsPtr exec(const QString &query, const QVariant &arg);

        /**
         * @brief Executes SQL query.
         * @overload SqlResultsPtr exec(const QString &query, std::initializer_list<QVariant> list)
         */
        SqlResultsPtr exec(const QString &query, std::initializer_list<QVariant> argList);

        /**
         * @brief Executes SQL query.
         * @overload SqlResultsPtr exec(const QString &query, std::initializer_list<std::pair<QString,QVariant>> argMap)
         */
        SqlResultsPtr exec(const QString &query, std::initializer_list<std::pair<QString,QVariant>> argMap);

        /**
         * @brief Executes SQL query asynchronously using list of parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: ?, :param, \@param, just don't mix different types in single query.
         * @param args List of parameter values to bind.
         * @param resultsHandler Function (can be lambda) to handle results. The function has to accept single SqlResultsPtr object and return nothing.
         * @param flags Execution flags. See exec() for setails.
         *
         * Asynchronous execution takes place in another thread. Once the execution is finished, the results handler function is called.
         *
         * Example:
         * @code
         * db->asyncExec("SELECT * FROM table WHERE col = ?", {5}, [=](SqlResultsPtr results)
         * {
         *     qDebug() << "Received" << results->rowCount() << "rows in results.";
         * });
         * @endcode
         */
        void asyncExec(const QString& query, const QList<QVariant>& args, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query asynchronously using named parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: :param, \@param, just don't mix different types in single query.
         * @param args Map of parameter name and the value assigned to it.
         * @param resultsHandler Function (can be lambda) to handle results. The function has to accept single SqlResultsPtr object and return nothing.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload void asyncExec(const QString& query, const QHash<QString, QVariant>& args, QueryResultsHandler resultsHandler, Flags flags)
         */
        void asyncExec(const QString& query, const QHash<QString, QVariant>& args, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query asynchronously.
         * @param query Query to be executed. See exec() for details.
         * @param resultsHandler Function (can be lambda) to handle results. The function has to accept single SqlResultsPtr object and return nothing.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload void asyncExec(const QString& query, QueryResultsHandler resultsHandler, Flags flags)
         */
        void asyncExec(const QString& query, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE);

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
        quint32 asyncExec(const QString& query, const QList<QVariant>& args, Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query asynchronously using named parameters.
         * @param query Query to be executed. Parameter placeholders can be either of: :param, \@param, just don't mix different types in single query.
         * @param args Map of parameter name and the value assigned to it.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload quint32 asyncExec(const QString& query, const QHash<QString, QVariant>& args, Flags flags)
         *
         * It's recommended to use method version which takes function pointer for results handing, as it's more resiliant to errors in the code.
         */
        quint32 asyncExec(const QString& query, const QHash<QString, QVariant>& args, Flags flags = Flag::NONE);

        /**
         * @brief Executes SQL query asynchronously.
         * @param query Query to be executed. See exec() for details.
         * @param flags Execution flags. See exec() for details.
         * @return Asynchronous execution ID.
         * @overload quint32 asyncExec(const QString& query, Flags flags)
         *
         * It's recommended to use method version which takes function pointer for results handing, as it's more resiliant to errors in the code.
         */
        quint32 asyncExec(const QString& query, Flags flags = Flag::NONE);

        /**
         * @brief Begins SQL transaction.
         * @return true on success, or false on failure.
         *
         * This method uses basic "BEGIN" statement to begin transaction, therefore recurrent transactions are not supported.
         * This is because SQLite2 doesn't support "SAVEPOINT" and this is the common interface for all SQLite versions.
         */
        bool begin();

        /**
         * @brief Commits SQL transaction.
         * @return true on success, or false otherwise.
         */
        bool commit();

        /**
         * @brief Rolls back the transaction.
         * @return true on success, or false otherwise (i.e. there was no transaction open, there was a connection problem, etc).
         */
        bool rollback();

        /**
         * @brief Interrupts current execution.
         *
         * This method makes sense only when execution takes place in thread other, than the one calling this method.
         * It interrupts execution - in most cases instantly. It calls sqlite*_interrupt(), so the actual behaviour is defined there.
         * This method doesn't return until the interrupting is done.
         */
        void interrupt();

        /**
         * @brief Interrupts current execution asynchronously.
         *
         * It's almost the same as interrupt(), except it returns immediately, instead of waiting for the interruption to finish.
         * In case of some heavy queries the interruption process might take a little while.
         */
        void asyncInterrupt();

        /**
         * @brief Checks if the database is readable at the moment.
         * @return true if the database is readable, or false otherwise.
         *
         * The database can be in 3 states: not locked, locked for reading or locked for writing.
         * If it's locked for writing, than it's not readable and this method will return false.
         * If it's locked for reading or not locked at all, then this method will return true.
         * Database can be locked by other threads executing their queries on the database.
         */
        bool isReadable();

        /**
         * @brief Checks if the database is writable at the moment.
         * @return true if the database is writable, or false otherwise.
         *
         * The database can be in 3 states: not locked, locked for reading or locked for writing.
         * If it's locked for writing (by other thread) or reading, than it's not writable and this method will return false.
         * If it's not locked at all, then this method will return true.
         * Database can be locked by other threads executing their queries on the database.
         */
        bool isWritable();

        /**
         * @brief Attaches given database to this database.
         * @param otherDb Other registered database object.
         * @return Name of the attached database (it's not the symbolic name of the other database, it's a name you would use in <tt>ATTACH 'name'</tt> statement).
         *
         * This is convinent method to attach other registered databases to this database. It generates attached database name, so it doesn't conflict
         * with other - already attached - database names, attaches the database with that name and returns that name to you, so you can refer to it in queries.
         */
        QString attach(Db* otherDb);

        /**
         * @brief Detaches given database from this database.
         * @param otherDb Other registered database object.
         *
         * If the otherDb is not attached, this method does nothing. Otherwise it calls <tt>DETACH</tt> statement using the attach name generated before by attach().
         * You don't have to provide the attach name, as Db class remembers those names internally.
         */
        void detach(Db* otherDb);

        /**
         * @brief Detaches all attached databases.
         *
         * Detaches all attached databases. This includes only databases attached with attach(). Databases attached with manual <tt>ATTACH</tt> query execution
         * will not be detached.
         */
        void detachAll();

        /**
         * @brief Gets attached databases.
         * @return Table of attached databases and the attach names used to attach them.
         *
         * This method returns only databases attached with attach() method.
         */
        const QHash<Db*,QString>& getAttachedDatabases();

        /**
         * @brief Gets all attached databases.
         * @return Set of attach names.
         *
         * This method returns all attached database names (the attach names), including both those from attach() and manual <tt>ATTACH</tt> query execution.
         */
        QSet<QString> getAllAttaches();

        /**
         * @brief Generates unique name for object to be created in the database.
         * @param attachedDbName Optional attach name, so the name will be in context of that database.
         * @return Unique object name.
         *
         * Queries database for all existing objects and then generates name that is not on that list.
         * The generated name is a random string of length 16.
         */
        QString getUniqueNewObjectName(const QString& attachedDbName = QString());

        /**
         * @brief Gets last error string from database driver.
         * @return Last encountered error.
         *
         * Result of this method is determinated by DbPlugin. For plugins using Qt database engine, this method calls QSqlDatabase::lastError().
         */
        QString getErrorText();

        /**
         * @brief Gets last error code from database driver.
         * @return Code of last encountered error.
         *
         * Result of this method is determinated by DbPlugin. For plugins using Qt database engine, this method calls QSqlDatabase::lastError().
         */
        int getErrorCode();

        /**
         * @brief Gets database type label.
         * @return Database type label.
         *
         * The database type label is used on UI to tell user what database it is (SQLite 3, SQLite 2, Encrypted SQLite 3, etc).
         * This is defined by DbPlugin.
         */
        virtual QString getTypeLabel() = 0;

    protected:
        /**
         * @brief Initializes database object.
         */
        Db();

        /**
         * @brief Generates unique database name for ATTACH.
         * @param lock Defines if the lock on dbOperLock mutex.
         * @return Unique database name.
         *
         * Database name here is the name to be used for ATTACH statement.
         * For example it will never be "main" or "temp", as those names are already used.
         * It also will never be any name that is currently used for any ATTACH'ed database.
         * It respects both manual ATTACH'es (called by user), as well as by attach() calls.
         *
         * Operations on database are normally locked during name generation, because it involves
         * queries to the database about what are currently existing objects.
         * The lock can be ommited if the calling method already locked dbOperLock.
         */
        QString generateUniqueDbName(bool lock = true);

        /**
         * @brief Detaches given database object from this database.
         * @param otherDb Other registered database.
         *
         * This is called from detach() and detachAll().
         */
        void detachInternal(Db* otherDb);

        /**
         * @brief Clears attached databases list.
         *
         * Called by closeQuiet(). Only clears maps and lists regarding attached databases.
         * It doesn't call detach(), because closing the database will already detach all databases.
         */
        void clearAttaches();

        /**
         * @brief Generated unique ID for asynchronous query execution.
         * @return Unique ID.
         */
        static quint32 generateAsyncId();

        /**
         * @brief Executes query asynchronously.
         * @param runner Prepared object for asynchronous execution.
         * @return Asynchronous execution unique ID.
         *
         * This is called by asyncExec(). Runs prepared runner object (which has all information about the query)
         * on separate thread.
         */
        quint32 asyncExec(AsyncQueryRunner* runner);

        /**
         * @brief Initializes database object with defined parameters.
         * @param name Database name. It will be also presented with this name to the user.
         * @param path Database file path.
         * @param options Custom options for the database (such as user, password, etc).
         * @return true on success, false on failure.
         */
        bool init(const QString &name, const QString &path, const QHash<QString,QVariant>& options);

        /**
         * @brief Opens the database and calls initial setup.
         * @return true on success, false on failure.
         *
         * Calls openInternal() and if it succeeded, calls initialDbSetup().
         * It's called from openQuiet().
         */
        bool openAndSetup();

        /**
         * @brief Initializes database object.
         * @return true on success, false on failure.
         *
         * Implementation of this method should perform initialization that is necessary after object creation.
         * This is after database name, file path and options were defined.
         */
        virtual bool init() = 0;

        /**
         * @brief Checks if the database connection is open.
         * @return true if the connection is open, or false otherwise.
         *
         * This is called from isOpen(). Implementation should test and return information if the database
         * connection is open. A lock on connectionStateLock is already set by the isOpen() method.
         */
        virtual bool isOpenInternal() = 0;

        /**
         * @brief Interrupts execution of any queries.
         *
         * Implementation of this method should interrupt any query executions that are currently in progress.
         * Typical implementation for SQLite databases will call sqlite_interupt() / sqlite3_interupt().
         */
        virtual void interruptExecution() = 0;

        /**
         * @brief Executes given query with parameters on the database.
         * @param query Query to execute.
         * @param args Query parameters.
         * @return Results from execution.
         *
         * Implementation of this method should execute given query on the database, given that the query string
         * can contain parameter placeholders, such as ?, :param, \@param, or $param. SQLite 3 API understands
         * these parameters, while SQLite 2 API understands only ? placeholders and others need to be emulated
         * by this method implementation.
         *
         * Note that the parameters for this method are passed as list, so it doesn't matter what are names
         * in named placeholders. Parameters should be bind by position, not name. For name-aware parameter binding
         * there is a overloaded execInternal() method with QHash of parameters.
         */
        virtual SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args) = 0;

        /**
         * @overload SqlResultsPtr execInternal(const QString& query, const QHash<QString,QVariant>& args)
         */
        virtual SqlResultsPtr execInternal(const QString& query, const QHash<QString,QVariant>& args) = 0;

        /**
         * @brief Returns error message.
         * @return Error string.
         *
         * This can be either error from last query execution, but also from connection opening problems, etc.
         */
        virtual QString getErrorTextInternal() = 0;

        /**
         * @brief Returns error code.
         * @return Error code.
         *
         * This can be either error from last query execution, but also from connection opening problems, etc.
         */
        virtual int getErrorCodeInternal() = 0;

        /**
         * @brief Opens database connection.
         * @return true on success, false on failure.
         *
         * Opens database. Called by open() and openAndSetup().
         */
        virtual bool openInternal() = 0;

        /**
         * @brief Closes database connection.
         *
         * Closes database. Called by open() and openQuiet().
         */
        virtual bool closeInternal() = 0;

        /**
         * @brief Initializes resources after database connection was open.
         *
         * Implementation of this method should setup any necessary options (such as PRAGMAs)
         * on the database. It's called each time the connection to the database is estabilished.
         *
         * It's called from inside of open(), so the dbOperLock is locked at the moment.
         * Use execNoLock() for any query executions in the implementation of this method.
         */
        virtual void initialDbSetup();

        virtual bool deregisterFunction(const QString& name, int argCount) = 0;
        virtual bool registerScalarFunction(const QString& name, int argCount) = 0;
        virtual bool registerAggregateFunction(const QString& name, int argCount) = 0;

        /**
         * @brief Database name.
         *
         * It must be unique across all Db instances. Use generateUniqueDbName() to get the unique name
         * for new database. It's used as a key for DbManager.
         *
         * Databases are also presented to the user with this name on UI.
         */
        QString name;

        /**
         * @brief Path to the database file.
         */
        QString path;

        /**
         * @brief Connection options.
         *
         * There are no standard options. Custom DbPlugin implementations may support some options.
         */
        QHash<QString,QVariant> connOptions;

        /**
         * @brief SQLite version of this database.
         *
         * This is only a major version number (2 or 3).
         */
        quint8 version = 0;

        /**
         * @brief Map of databases attached to this database.
         *
         * It's mapping from ATTACH name to the database object. It contains only attaches
         * that were made with attach() calls.
         */
        QHash<QString,Db*> attachedDbMap;

        /**
         * @brief Map of databases attached to this database.
         *
         * This is an inversion of attachedDbMap.
         */
        QHash<Db*,QString> attachedDbNameMap; // TODO replace attachedDbMap and attachedDbNameMap with BiDiMap.

        /**
         * @brief Counter of attaching requrests for each database.
         *
         * When calling attach() on other Db, it gets its own entry in this mapping.
         * If the mapping already exists, its value is incremented.
         * Then, when calling detach(), counter is decremented and when it reaches 0,
         * the database is actualy detached.
         */
        QHash<Db*,int> attachCounter;

        /**
         * @brief Result handler functions for asynchronous executions.
         *
         * For each asyncExec() with function pointer in argument there's an entry in this map
         * pointing to the function. Keys are asynchronous IDs.
         */
        QHash<int,QueryResultsHandler> resultHandlers;

    private:
        /**
         * @brief Applies execution flags and executes query.
         * @param query Query to be executed.
         * @param args Query parameters.
         * @param flags Query execution flags.
         * @return Execution results - either successful or failed.
         *
         * This is called from both exec() and execNoLock() and is a final step before calling execInternal()
         * (the plugin-provided execution). This is where \p flags are interpreted and applied.
         */
        SqlResultsPtr execHashArg(const QString& query, const QHash<QString, QVariant>& args, Flags flags);

        /**
         * @overload SqlResultsPtr execListArg(const QString& query, const QList<QVariant>& args, Flags flags)
         */
        SqlResultsPtr execListArg(const QString& query, const QList<QVariant>& args, Flags flags);

        /**
         * @brief Generates unique database name.
         * @return Unique database name.
         *
         * This is a lock-less variant of generateUniqueDbName(). It is called from that method.
         * See generateUniqueDbName() for details.
         */
        QString generateUniqueDbNameNoLock();

        /**
         * @brief Provides required locking mode for given query.
         * @param query Query to be executed.
         * @return Locking mode: READ or WRITE.
         *
         * Given the query this method analyzes what is the query and provides information if the query
         * will do some changes on the database, or not. Then it returns proper locking mode that should
         * be used for this query execution.
         *
         * Query execution methods from this class check if lock mode of the query to be executed isn't
         * in conflict with the lock being currently applied on the dbOperLock (if any is applied at the moment).
         *
         * This method works on a very simple rule. It assumes that queries: SELECT, ANALYZE, EXPLAIN,
         * and PRAGMA - are read-only, while all other queries are read-write.
         * In case of PRAGMA this is not entirely true, but it's not like using PRAGMA for changing
         * some setting would cause database state inconsistency. At least not from perspective of SQLiteStudio.
         */
        ReadWriteLocker::Mode getLockingMode(const QString& query, Db::Flags flags);

        /**
         * @brief Handles asynchronous query results with results handler function.
         * @param asyncId Asynchronous ID.
         * @param results Results from execution.
         * @return true if the results were handled, or false if they were not.
         *
         * This method checks if there is a handler function for given asynchronous ID (in resultHandlers)
         * and if there is, then evaluates it and returns true. Otherwise does nothing and returns false.
         */
        bool handleResultInternally(quint32 asyncId, SqlResultsPtr results);

        /**
         * @brief Database operation lock.
         *
         * This lock is set whenever any operation on the actual database is performed (i.e. call to
         * exec(), interrupt(), open(), close(), generateUniqueDbName(true),  attach(), detach(), and others...
         * generally anything that does operations on database that must be synchronous).
         *
         * In case of exec() it can be locked for READ or WRITE (depending on query type),
         * because there can be multiple SELECTs and there's nothing wrong with it,
         * while for other methods is always lock for WRITE.
         */
        QReadWriteLock dbOperLock;

        /**
         * @brief Connection state lock.
         *
         * It's locked whenever the connection state is changed or tested.
         * For open() and close() it's a WRITE lock, for isOpen() it's READ lock.
         */
        QReadWriteLock connectionStateLock;

        /**
         * @brief Sequence container for generating unique asynchronous IDs.
         */
        static quint32 asyncId;

        /**
         * @brief Most recent error message.
         *
         * This is local storage for last error from last SqlResultsPtr.
         * It is necessary to store that, because it looks like QSqlDatabase
         * doesn't provide last error text in case when QSqlResults carried that text for it.
         * It resulted in empty "lastErrorText()" results, while the actual error text
         * was returned in SqlResultsPtr.
         */
        QString lastErrorText;

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
        void asyncExecFinished(quint32 asyncId, SqlResultsPtr results);

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

    private slots:
        /**
         * @brief Handles asynchronous execution results.
         * @param runner Container with input and output data of the query.
         *
         * This is called from the other thread when it finished asynchronous execution.
         * It checks if there is any handler function to evaluate it with results
         * and if there's not, emits asyncExecFinished() signal.
         */
        void asyncQueryFinished(AsyncQueryRunner* runner);

    public slots:
        /**
         * @brief Opens connection to the database.
         * @return true on success, false on error.
         *
         * Emits connected() only on success.
         */
        bool open();

        /**
         * @brief Closes connection to the database.
         * @return true on success, false on error.
         *
         * Emits disconnected() only on success (i.e. db was open before).
         */
        bool close();

        /**
         * @brief Opens connection to the database quietly.
         * @return true on success, false on error.
         *
         * Opens database, doesn't emit any signal.
         */
        bool openQuiet();

        /**
         * @brief Closes connection to the database quietly.
         * @return true on success, false on error.
         *
         * Closes database, doesn't emit any signal.
         */
        bool closeQuiet();

};

QDataStream &operator<<(QDataStream &out, const Db* myObj);
QDataStream &operator>>(QDataStream &in, Db*& myObj);

Q_DECLARE_METATYPE(Db*)
Q_DECLARE_OPERATORS_FOR_FLAGS(Db::Flags)

#endif // DB_H
