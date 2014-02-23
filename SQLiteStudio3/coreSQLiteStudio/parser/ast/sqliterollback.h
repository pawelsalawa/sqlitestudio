#ifndef SQLITEROLLBACK_H
#define SQLITEROLLBACK_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteRollback : public SqliteQuery
{
    public:
        SqliteRollback();
        SqliteRollback(bool transactionKw, const QString& name);
        SqliteRollback(bool transactionKw, bool savePoint, const QString& name);

        bool transactionKw = false;
        bool toKw = false;
        bool savepointKw = false;
        QString name = QString::null;
};

typedef QSharedPointer<SqliteRollback> SqliteRollPtr;

#endif // SQLITEROLLBACK_H
