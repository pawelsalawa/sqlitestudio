#ifndef DBSQLITEWXINSTANCE_H
#define DBSQLITEWXINSTANCE_H

#include "db/abstractdb3.h"
#include "wxsqlite3.h"
#include "db/stdsqlite3driver.h"

STD_SQLITE3_DRIVER(WxSQLite, "WxSQLite3", wx_,)

class DbSqliteWxInstance : public AbstractDb3<WxSQLite>
{
    public:
        DbSqliteWxInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &connOptions);

    protected:
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBSQLITEWXINSTANCE_H
