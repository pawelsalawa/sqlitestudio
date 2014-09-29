#ifndef SQLITECREATEVIEW_H
#define SQLITECREATEVIEW_H

#include "sqlitequery.h"
#include <QString>

class SqliteSelect;

class API_EXPORT SqliteCreateView : public SqliteQuery
{
    public:
        SqliteCreateView();
        SqliteCreateView(const SqliteCreateView& other);
        SqliteCreateView(int temp, bool ifNotExists, const QString& name1, const QString& name2, SqliteSelect* select);
        ~SqliteCreateView();

        SqliteStatement* clone();

        bool tempKw = false;
        bool temporaryKw = false;
        bool ifNotExists = false;
        QString database = QString::null;
        QString view = QString::null;
        SqliteSelect* select = nullptr;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteCreateView> SqliteCreateViewPtr;

#endif // SQLITECREATEVIEW_H
