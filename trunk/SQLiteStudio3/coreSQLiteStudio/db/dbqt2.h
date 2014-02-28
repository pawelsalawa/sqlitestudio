#ifndef DBQT2_H
#define DBQT2_H

#include "coreSQLiteStudio_global.h"
#include "dbqt.h"

/**
 * @brief Variation of DbQt for SQLite 2.
 *
 * Inherit this when implementing Db for SQLite 2.
 * See DbQt for more details.
 */
class API_EXPORT DbQt2 : public DbQt
{
    public:
        /**
         * @brief Creates database object based on Qt database framework.
         * @param driverName Driver names as passed to QSqlDatabase::addDatabase().
         * @param type Database type (SQLite3, SQLite2 or other...) used as a database type presented to user.
         *
         * All values from this constructor are just passed to DbQt constructor.
         */
        DbQt2(const QString& driverName, const QString& type);

        /**
         * @brief Common internal execution routing for SQLite 2.
         * @param query Query to be executed.
         * @param args Arguments for query.
         * @return Execution results.
         *
         * This is a replacement method for the regular execInternal(). It adds named parameter placeholders support
         * for SQLite 2, which normally doesn't support them.
         */
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args);

        /**
         * @overload SqlResultsPtr execInternal(const QString &query, const QHash<QString, QVariant> &args)
         */
        SqlResultsPtr execInternal(const QString& query, const QHash<QString,QVariant>& args);
};

#endif // DBQT2_H
