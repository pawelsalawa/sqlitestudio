#ifndef SQLITEDROPVIEW_H
#define SQLITEDROPVIEW_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropView : public SqliteQuery
{
    public:
        SqliteDropView();
        SqliteDropView(bool ifExistsKw, const QString& name1, const QString& name2);

        bool ifExistsKw = false;
        QString database = QString::null;
        QString view = QString::null;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
};

typedef QSharedPointer<SqliteDropView> SqliteDropViewPtr;

#endif // SQLITEDROPVIEW_H
