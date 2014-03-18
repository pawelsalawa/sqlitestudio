#ifndef ABSTRACTDB_H
#define ABSTRACTDB_H

#include "returncode.h"
#include "sqlresults.h"
#include "dialect.h"
#include "db/db.h"
#include "services/functionmanager.h"
#include "common/readwritelocker.h"
#include "coreSQLiteStudio_global.h"
#include <QObject>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QSet>
#include <QReadWriteLock>
#include <QRunnable>
#include <QStringList>

class AsyncQueryRunner;

/**
 * @brief Base database logic implementation.
 *
 * This class implements common base logic for all database implementations. It's still abstract class
 * and needs further implementation to be usable.
 */
class API_EXPORT AbstractDb : public Db
{
    Q_OBJECT

    public:
        /**
         * @brief Initializes database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See below for details.
         *
         * Connection options are handled individually by the derived database implementation class.
         * It can be password for encrypted databases, read-only access flag, etc.
         */
        AbstractDb(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        virtual ~AbstractDb();

        bool isOpen();
        QString getName();
        QString getPath();
        quint8 getVersion();
        Dialect getDialect();
        QString getEncoding();
        QHash<QString,QVariant>& getConnectionOptions();
        void setName(const QString& value);
        void setPath(const QString& value);
        void setConnectionOptions(const QHash<QString,QVariant>& value);
        SqlResultsPtr exec(const QString& query, const QList<QVariant> &args, Flags flags = Flag::NONE);
        SqlResultsPtr exec(const QString& query, const QHash<QString, QVariant>& args, Flags flags = Flag::NONE);
        SqlResultsPtr exec(const QString &query, Db::Flags flags = Flag::NONE);
        SqlResultsPtr exec(const QString &query, const QVariant &arg);
        SqlResultsPtr exec(const QString &query, std::initializer_list<QVariant> argList);
        SqlResultsPtr exec(const QString &query, std::initializer_list<std::pair<QString,QVariant>> argMap);
        void asyncExec(const QString& query, const QList<QVariant>& args, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE);
        void asyncExec(const QString& query, const QHash<QString, QVariant>& args, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE);
        void asyncExec(const QString& query, QueryResultsHandler resultsHandler, Flags flags = Flag::NONE);
        quint32 asyncExec(const QString& query, const QList<QVariant>& args, Flags flags = Flag::NONE);
        quint32 asyncExec(const QString& query, const QHash<QString, QVariant>& args, Flags flags = Flag::NONE);
        quint32 asyncExec(const QString& query, Flags flags = Flag::NONE);
        bool begin();
        bool commit();
        bool rollback();
        void interrupt();
        void asyncInterrupt();
        bool isReadable();
        bool isWritable();
        QString attach(Db* otherDb);
        void detach(Db* otherDb);
        void detachAll();
        const QHash<Db*,QString>& getAttachedDatabases();
        QSet<QString> getAllAttaches();
        QString getUniqueNewObjectName(const QString& attachedDbName = QString());
        QString getErrorText();
        int getErrorCode();
        bool initAfterCreated();
        void setTimeout(int secs);
        int getTimeout() const;

    protected:
        struct FunctionUserData
        {
            QString name;
            int argCount = 0;
            Db* db = nullptr;
        };

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
         * @brief Opens the database and calls initial setup.
         * @return true on success, false on failure.
         *
         * Calls openInternal() and if it succeeded, calls initialDbSetup().
         * It's called from openQuiet().
         */
        bool openAndSetup();

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
        virtual void initAfterOpen();

        static QHash<QString,QVariant> getAggregateContext(void* memPtr);
        static void setAggregateContext(void* memPtr, const QHash<QString,QVariant>& aggregateContext);
        static void releaseAggregateContext(void* memPtr);

        /**
         * @brief Evaluates requested function using defined implementation code and provides result.
         * @param dataPtr SQL function user data (defined when registering function). Must be of FunctionUserData* type, or descendant.
         * @param argList List of arguments passed to the function.
         * @param[out] ok true (default) to indicate successful execution, or false to report an error.
         * @return Result returned from the plugin handling function implementation.
         *
         * This method is aware of the implementation language and the code defined for it,
         * so it delegates the execution to the proper plugin handling that language.
         *
         * This method is called for scalar functions.
         */
        static QVariant evaluateScalar(void* dataPtr, const QList<QVariant>& argList, bool& ok);
        static void evaluateAggregateStep(void* dataPtr, QHash<QString, QVariant>& aggregateContext, QList<QVariant> argList);
        static QVariant evaluateAggregateFinal(void* dataPtr, QHash<QString, QVariant>& aggregateContext, bool& ok);

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
         * @brief Represents single function that is registered in the database.
         *
         * Registered custom SQL functions are diversed by SQLite by their name, arguments count and their type,
         * so this structure has exactly those parameters.
         */
        struct RegisteredFunction
        {
            /**
             * @brief Function name.
             */
            QString name;

            /**
             * @brief Arguments count (-1 for undefined count).
             */
            int argCount;

            /**
             * @brief Function type.
             */
            FunctionManager::Function::Type type;
        };

        friend int qHash(const AbstractDb::RegisteredFunction& fn);
        friend bool operator==(const AbstractDb::RegisteredFunction& fn1, const AbstractDb::RegisteredFunction& fn2);

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
         * @brief Registers single custom SQL function.
         * @param function Function to register.
         *
         * If function got registered successfly, it's added to registeredFunctions.
         * If there was a function with the same name, argument count and type already registered,
         * it will be overwritten (both in SQLite and in registeredFunctions).
         */
        void registerFunction(const RegisteredFunction& function);

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
         * @brief Current timeout (in seconds) for waiting for the database to be released from the lock.
         *
         * See Db::setTimeout() for details.
         */
        int timeout = 60;

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

        /**
         * @brief List of all functions currently registered in this database.
         */
        QSet<RegisteredFunction> registeredFunctions;

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

        /**
         * @brief Deregisters all funtions registered in the database and registers new (possibly the same) functions.
         *
         * This slot is called from openAndSetup() and then every time user modifies custom SQL functions and commits changes to them.
         * It deregisters all functions registered before in this database and registers new functions, currently defined for
         * this database.
         *
         * @see FunctionManager
         */
        void registerAllFunctions();
};

/**
 * @brief Standard function required by QHash.
 * @param fn Function to calculate hash for.
 * @return Hash value calculated from all members of DbBase::RegisteredFunction.
 */
int qHash(const AbstractDb::RegisteredFunction& fn);

/**
 * @brief Simple comparator operator, compares all members.
 * @param other Other function to compare.
 * @return true if \p other is equal, false otherwise.
 *
 * This function had to be declared/defined outside of the DbBase::RegisteredFunction, because QSet/QHash requires this.
 */
bool operator==(const AbstractDb::RegisteredFunction& fn1, const AbstractDb::RegisteredFunction& fn2);

#endif // ABSTRACTDB_H
