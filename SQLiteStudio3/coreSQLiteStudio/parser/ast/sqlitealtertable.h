#ifndef SQLITEALTERTABLE_H
#define SQLITEALTERTABLE_H

#include "sqlitequery.h"
#include "sqlitecreatetable.h"

class API_EXPORT SqliteAlterTable : public SqliteQuery
{
    Q_OBJECT

    public:
        enum class Command
        {
            RENAME,
            ADD_COLUMN,
            DROP_COLUMN,
            null
        };

        SqliteAlterTable();
        SqliteAlterTable(const SqliteAlterTable& other);
        SqliteAlterTable(const QString& name1, const QString& name2, const QString& newName);
        SqliteAlterTable(const QString& name1, const QString& name2, bool columnKw, SqliteCreateTable::Column* column);
        SqliteAlterTable(const QString& name1, const QString& name2, bool columnKw, const QString& dropColumn);
        ~SqliteAlterTable();
        SqliteStatement* clone();

    protected:
        QStringList getColumnsInStatement();
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getColumnTokensInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void initName(const QString& name1, const QString& name2);

    public:
        Command command = Command::null;
        QString newName = QString();
        QString database = QString();
        QString table = QString();
        QString dropColumnName = QString();
        bool columnKw = false;
        SqliteCreateTable::Column* newColumn = nullptr;
};

typedef QSharedPointer<SqliteAlterTable> SqliteAlterTablePtr;

#endif // SQLITEALTERTABLE_H
