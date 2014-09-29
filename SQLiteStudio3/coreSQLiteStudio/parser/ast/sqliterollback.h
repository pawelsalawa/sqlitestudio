#ifndef SQLITEROLLBACK_H
#define SQLITEROLLBACK_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteRollback : public SqliteQuery
{
    public:
        SqliteRollback();
        SqliteRollback(const SqliteRollback& other);
        SqliteRollback(bool transactionKw, const QString& name);
        SqliteRollback(bool transactionKw, bool savePoint, const QString& name);

        SqliteStatement* clone();

        bool transactionKw = false;
        bool toKw = false;
        bool savepointKw = false;
        QString name = QString::null;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteRollback> SqliteRollPtr;

#endif // SQLITEROLLBACK_H
