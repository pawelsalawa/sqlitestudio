#ifndef DBQT_H
#define DBQT_H

#include "db.h"
#include "../returncode.h"
#include "sqlresults.h"
#include "../dialect.h"
#include "dbpluginqt.h"

#include <QObject>
#include <QSqlDatabase>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QHash>
#include <QMutex>
#include <QRunnable>

/**
 * @brief The Db class implemented with Qt's database framework.
 *
 * Use DbQt2 or DbQt3 if you don't want to implement whole Db interface by yourself
 * and your implementation can rely on QtSql module. DbQt2 and DbQt3 inherit from this class.
 *
 * It implements most of the necessary methods of Db interface.
 * It still an abstract class and you will have to inherit and implement DbQt2 or DbQt3,
 * but it's way simpler than implementing entire Db interface.
 *
 * Don't inherit DbQt directly. Instead inherit DbQt2 for SQLite2 implementation
 * and DbQt3 for SQLite3 implementation. They provide some implementation specialized
 * to their SQLite versions.
 */
class API_EXPORT DbQt : public Db
{
    Q_OBJECT

    public:
        /**
         * @brief Releases resources.
         *
         * Detaches attached databases, closes the database (if open)
         * and removes this database from Qt's databases registry.
         */
        ~DbQt();

        QString getTypeLabel();

    protected:
        struct FunctionUserData
        {
            QString name;
            int argCount = 0;
            Db* db = nullptr;
        };

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
         * @brief Creates database object based on Qt database framework.
         * @param driverName Driver names as passed to QSqlDatabase::addDatabase().
         * @param type Database type (SQLite3, SQLite2 or other...) used as a database type presented to user.
         */
        DbQt(const QString& driverName, const QString& type);

        bool isOpenInternal();
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args);
        SqlResultsPtr execInternal(const QString& query, const QHash<QString,QVariant>& args);
        bool init();
        bool openInternal();
        bool closeInternal();
        void interruptExecution();
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount);
        bool registerAggregateFunction(const QString& name, int argCount);

        virtual bool deregisterFunction(const QVariant& handle, const QString& name, int argCount) = 0;
        virtual bool registerScalarFunction(const QVariant& handle, const QString& name, int argCount) = 0;
        virtual bool registerAggregateFunction(const QVariant& handle, const QString& name, int argCount) = 0;

        /**
         * @brief Executes sqlite interrupt on the database handle.
         * @param handle Database handle (of sqlite* or sqlite3* type).
         *
         * The handle is extracted from QSqlDriver.
         *
         * Implementation of this method should cast the handle to sqlite* or sqlite3* type
         * and call sqlite_interrupt() or sqlite3_interrupt() on it.
         * Since interrupting depends on the sqlite version, it has to be implemented in the DbPlugin.
         *
         * Casting can be dony with:
         * @code
         * sqlite* sqliteHnd = *static_cast<sqlite**>(const_cast<void*>(handle.data()));
         * @endcode
         */
        virtual void interruptExecutionOnHandle(const QVariant& handle) = 0;

        /**
         * @brief Name of the driver, as passed to constructor.
         */
        QString driverName;

        /**
         * @brief Database type, as passed to constructor.
         */
        QString type;

        /**
         * @brief Pointer to Qt's database object.
         */
        QSqlDatabase* db = nullptr;
};

#endif // DBQT_H
