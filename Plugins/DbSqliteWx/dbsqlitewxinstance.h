#ifndef DBSQLITEWXINSTANCE_H
#define DBSQLITEWXINSTANCE_H

#include "db/abstractdb3.h"

#ifdef WXSQLITE_SYSTEM_LIB
#  include "wxsqlite3_unmodified.h"
#else
#  include "wxsqlite3.h"
#endif

#include "db/stdsqlite3driver.h"

#ifdef WXSQLITE_SYSTEM_LIB
STD_SQLITE3_DRIVER(WxSQLite, "WxSQLite3",,)
#else
STD_SQLITE3_DRIVER(WxSQLite, "WxSQLite3", wx_,)
#endif

class DbSqliteWxInstance : public AbstractDb3<WxSQLite>
{
    public:
        DbSqliteWxInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &connOptions);

        Db* clone() const;
        QString getTypeClassName() const;

    protected:
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBSQLITEWXINSTANCE_H
