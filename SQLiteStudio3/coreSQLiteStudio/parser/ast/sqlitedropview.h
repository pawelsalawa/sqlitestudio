#ifndef SQLITEDROPVIEW_H
#define SQLITEDROPVIEW_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropView : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteDropView();
        SqliteDropView(const SqliteDropView& other);
        SqliteDropView(bool ifExistsKw, const QString& name1, const QString& name2);

        SqliteStatement* clone();

        bool ifExistsKw = false;
        QString database = QString();
        QString view = QString();

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDropView> SqliteDropViewPtr;

#endif // SQLITEDROPVIEW_H
