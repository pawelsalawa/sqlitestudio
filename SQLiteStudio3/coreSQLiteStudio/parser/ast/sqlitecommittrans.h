#ifndef SQLITECOMMITTRANS_H
#define SQLITECOMMITTRANS_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteCommitTrans : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteCommitTrans();
        SqliteCommitTrans(bool transactionKw, const QString &name, bool endKw);
        SqliteCommitTrans(const SqliteCommitTrans& other);
        SqliteStatement* clone();

        bool endKw = false;
        QString name = QString();
        bool transactionKw = false;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteCommitTrans> SqliteCommitTransPtr;

#endif // SQLITECOMMITTRANS_H
