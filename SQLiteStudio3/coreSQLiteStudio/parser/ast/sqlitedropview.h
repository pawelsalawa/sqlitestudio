#ifndef SQLITEDROPVIEW_H
#define SQLITEDROPVIEW_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropView : public SqliteQuery
{
    public:
        SqliteDropView();
        SqliteDropView(const SqliteDropView& other);
        SqliteDropView(bool ifExistsKw, const QString& name1, const QString& name2);

        SqliteStatement* clone();

        bool ifExistsKw = false;
        QString database = QString::null;
        QString view = QString::null;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDropView> SqliteDropViewPtr;

#endif // SQLITEDROPVIEW_H
