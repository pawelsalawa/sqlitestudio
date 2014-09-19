#ifndef SQLITEALTERTABLE_H
#define SQLITEALTERTABLE_H

#include "sqlitequery.h"
#include "sqlitecreatetable.h"

class API_EXPORT SqliteAlterTable : public SqliteQuery
{
    public:
        enum class Command
        {
            RENAME,
            ADD_COLUMN,
            null
        };

        SqliteAlterTable();
        SqliteAlterTable(const QString& name1, const QString& name2, const QString& newName);
        SqliteAlterTable(const QString& name1, const QString& name2, bool columnKw, SqliteCreateTable::Column* column);
        ~SqliteAlterTable();

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void initName(const QString& name1, const QString& name2);

    public:

        Command command = Command::null;
        QString newName = QString::null;
        QString database = QString::null;
        QString table = QString::null;
        bool columnKw = false;
        SqliteCreateTable::Column* newColumn = nullptr;
};

typedef QSharedPointer<SqliteAlterTable> SqliteAlterTablePtr;

#endif // SQLITEALTERTABLE_H
