#ifndef DBSQLITE3_H
#define DBSQLITE3_H

#include "abstractdb3.h"
#include "stdsqlite3driver.h"
#include "db/sqlite3.h"

STD_SQLITE3_DRIVER(Sqlite3, "SQLite 3",,)

class API_EXPORT DbSqlite3 : public AbstractDb3<Sqlite3>
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb3 constructor.
         */
        DbSqlite3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @overload
         */
        DbSqlite3(const QString& name, const QString& path);

        static bool complete(const QString& sql);
        static bool isDbFile(const QString& path);

        Db* clone() const;
        QString getTypeClassName() const;
};

#endif // DBSQLITE3_H
