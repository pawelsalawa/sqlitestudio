#ifndef DBQT3_H
#define DBQT3_H

#include "dbqt.h"

/**
 * @brief Variation of DbQt for SQLite 3.
 *
 * Inherit this when implementing Db for SQLite 3.
 * See DbQt for more details.
 */
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
        DbQt3(const QString& driverName, const QString& type);
};

#endif // DBQT3_H
