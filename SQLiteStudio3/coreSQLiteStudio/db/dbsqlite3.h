#ifndef DBSQLITE3_H
#define DBSQLITE3_H

#include "abstractdb3.h"
#include "common/global.h"

struct Sqlite3
{
    static_char* label = "SQLite 3";
};

class DbSqlite3 : public AbstractDb3<Sqlite3>
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
};

#endif // DBSQLITE3_H
