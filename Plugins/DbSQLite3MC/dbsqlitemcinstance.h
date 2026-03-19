#ifndef DBSQLITEMCINSTANCE_H
#define DBSQLITEMCINSTANCE_H

#include "db/abstractdb3.h"

#ifdef SQLITE3MC_SYSTEM_LIB
#  include "sqlite3mc_amalgamation_unmodified.h"
#else
#  include "sqlite3mc_amalgamation.h"
#endif

#include "db/stdsqlite3driver.h"

#ifdef SQLITE3MC_SYSTEM_LIB
STD_SQLITE3_DRIVER(SQLite3MC, "SQLite3MC",,)
#else
STD_SQLITE3_DRIVER(SQLite3MC, "SQLite3MC", mc_,)
#endif

class DbSqliteMcInstance : public AbstractDb3<SQLite3MC>
{
    public:
        DbSqliteMcInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &connOptions);

        Db* clone() const;
        QString getTypeClassName() const;
        QString getTypeLabel() const;

    protected:
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBSQLITEMCINSTANCE_H
