#ifndef SQLITECREATEVIRTUALTABLE_H
#define SQLITECREATEVIRTUALTABLE_H

#include "sqlitequery.h"

#include <QString>
#include <QList>

class API_EXPORT SqliteCreateVirtualTable : public SqliteQuery
{
    public:
        SqliteCreateVirtualTable();
        SqliteCreateVirtualTable(const SqliteCreateVirtualTable& other);
        SqliteCreateVirtualTable(bool ifNotExists, const QString& name1, const QString& name2,
                                 const QString& name3);
        SqliteCreateVirtualTable(bool ifNotExists, const QString& name1, const QString& name2,
                                 const QString& name3, const QList<QString>& args);

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();

    private:
        void initName(const QString& name1, const QString& name2);

    public:
        bool ifNotExistsKw = false;
        QString database = QString::null;
        QString table = QString::null;
        QString module = QString::null;
        QList<QString> args;
};

typedef QSharedPointer<SqliteCreateVirtualTable> SqliteCreateVirtualTablePtr;

#endif // SQLITECREATEVIRTUALTABLE_H
