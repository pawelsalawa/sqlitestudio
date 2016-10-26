#ifndef DBSQLITEWXINSTANCE_H
#define DBSQLITEWXINSTANCE_H

#include "db/abstractdb3.h"
#include "systemdata_sqlite3.h"
#include "db/stdsqlite3driver.h"

STD_SQLITE3_DRIVER(SystemDataSQLite, "System.Data.SQLite", systemdata_,)

class DbSqliteSystemDataInstance : public AbstractDb3<SystemDataSQLite>
{
    public:
        DbSqliteSystemDataInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &connOptions);

    protected:
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBSQLITEWXINSTANCE_H
