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
 * Use this class if you don't want to implement whole Db interface by yourself
 * and your implementation can rely on QtSql module.
 *
 * It implements all necessary methods of Db interface.
 * It still an abstract class and you will have to inherit and implement it,
 * but it's way simpler than implementing entire Db interface.
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

        /**
         * @brief Common internal execution routing for SQLite 2.
         * @param query Query to be executed.
         * @param args Arguments for query.
         * @return Execution results.
         *
         * This is a replacement method for the regular execInternal(), except it should be called
         * from DbQt instances implementing SQLite 2. It adds named parameter placeholders support
         * for SQLite 2, which normally doesn't support them.
         *
         * The usual usecase would be to reimplement execInternal() in the derived class and
         * as an implementation call this method. For example:
         * @code
         * SqlResultsPtr DbSqlite2Instance::execInternal(const QString& query, const QList<QVariant>& args)
         * {
         *     return DbQt::execInternalSqlite2(newQuery, args);
         * }
         *
         * SqlResultsPtr DbSqlite2Instance::execInternal(const QString& query, const QHash<QString, QVariant>& args)
         * {
         *     return DbQt::execInternalSqlite2(newQuery, newArgs);
         * }
         * @endcode
         */
        SqlResultsPtr execInternalSqlite2(const QString &query, const QList<QVariant> &args);

        /**
         * @overload SqlResultsPtr execInternalSqlite2(const QString &query, const QHash<QString, QVariant> &args)
         */
        SqlResultsPtr execInternalSqlite2(const QString &query, const QHash<QString, QVariant> &args);

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
