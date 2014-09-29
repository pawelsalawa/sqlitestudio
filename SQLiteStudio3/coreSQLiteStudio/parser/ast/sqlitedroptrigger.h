#ifndef SQLITEDROPTRIGGER_H
#define SQLITEDROPTRIGGER_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropTrigger : public SqliteQuery
{
    public:
        SqliteDropTrigger();
        SqliteDropTrigger(const SqliteDropTrigger& other);
        SqliteDropTrigger(bool ifExistsKw, const QString& name1, const QString& name2);

        SqliteStatement* clone();

        bool ifExistsKw = false;
        QString database = QString::null;
        QString trigger = QString::null;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDropTrigger> SqliteDropTriggerPtr;

#endif // SQLITEDROPTRIGGER_H
