#ifndef DBSQLITE2INSTANCE_H
#define DBSQLITE2INSTANCE_H

#include "db/abstractdb2.h"
#include "common/global.h"
#include <sqlite.h>

struct Sqlite2
{
    static_char* label = "SQLite 2";
};

class DbSqlite2Instance : public AbstractDb2<Sqlite2>
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb2 constructor.
         */
        DbSqlite2Instance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);
};

#endif // DBSQLITE2INSTANCE_H
