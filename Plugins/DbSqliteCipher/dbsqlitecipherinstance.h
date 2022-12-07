#ifndef DBSQLITECIPHERINSTANCE_H
#define DBSQLITECIPHERINSTANCE_H

#include "db/abstractdb3.h"

#ifdef SQLCIPHER_SYSTEM_LIB
#  include "sqlcipher_unmodified.h"
#else
#  include "sqlcipher.h"
#endif

#include "db/stdsqlite3driver.h"

#ifdef SQLCIPHER_SYSTEM_LIB
STD_SQLITE3_DRIVER(SqlCipher, "SQLCipher",,)
#else
STD_SQLITE3_DRIVER(SqlCipher, "SQLCipher", sqlcipher_,)
#endif

class DbSqliteCipherInstance : public AbstractDb3<SqlCipher>
{
    public:
        DbSqliteCipherInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        Db* clone() const;
        QString getTypeClassName() const;

    protected:
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBSQLITECIPHERINSTANCE_H
