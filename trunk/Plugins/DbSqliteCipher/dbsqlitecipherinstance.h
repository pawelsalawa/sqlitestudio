#ifndef DBSQLITECIPHERINSTANCE_H
#define DBSQLITECIPHERINSTANCE_H

#include "db/abstractdb3.h"
#include "sqlcipher.h"
#include "db/stdsqlite3driver.h"

STD_SQLITE3_DRIVER(SqlCipher, "SQLCipher", sqlcipher_,)

class DbSqliteCipherInstance : public AbstractDb3<SqlCipher>
{
    public:
        DbSqliteCipherInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

    protected:
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBSQLITECIPHERINSTANCE_H
