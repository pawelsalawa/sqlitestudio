#ifndef SQLITEROLLBACK_H
#define SQLITEROLLBACK_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteRollback : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteRollback();
        SqliteRollback(const SqliteRollback& other);
        SqliteRollback(bool transactionKw, const QString& name);
        SqliteRollback(bool transactionKw, bool savePoint, const QString& name);

        SqliteStatement* clone();

        bool transactionKw = false;
        bool toKw = false;
        bool savepointKw = false;
        QString name = QString();

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteRollback> SqliteRollPtr;

#endif // SQLITEROLLBACK_H
