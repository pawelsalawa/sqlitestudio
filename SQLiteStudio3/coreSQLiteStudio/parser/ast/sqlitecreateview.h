#ifndef SQLITECREATEVIEW_H
#define SQLITECREATEVIEW_H

#include "sqliteddlwithdbcontext.h"
#include "sqlitequery.h"
#include <QList>
#include <QString>

class SqliteSelect;
class SqliteIndexedColumn;

class API_EXPORT SqliteCreateView : public SqliteQuery, public SqliteDdlWithDbContext
{
    public:
        SqliteCreateView();
        SqliteCreateView(const SqliteCreateView& other);
        SqliteCreateView(int temp, bool ifNotExists, const QString& name1, const QString& name2, SqliteSelect* select);
        SqliteCreateView(int temp, bool ifNotExists, const QString& name1, const QString& name2, SqliteSelect* select, const QList<SqliteIndexedColumn*>& columns);
        ~SqliteCreateView();

        SqliteStatement* clone();
        QString getTargetDatabase() const;
        void setTargetDatabase(const QString& database);
        QString getObjectName() const;
        void setObjectName(const QString& name);
        TokenList equivalentSelectTokens() const;

        bool tempKw = false;
        bool temporaryKw = false;
        bool ifNotExists = false;
        QString database = QString();
        QString view = QString();
        SqliteSelect* select = nullptr;
        QList<SqliteIndexedColumn*> columns;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteCreateView> SqliteCreateViewPtr;

#endif // SQLITECREATEVIEW_H
