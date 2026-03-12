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
            RENAME_COLUMN,
            SET_NOT_NULL,
            DROP_NOT_NULL,
            ADD_CHECK,
            DROP_CONSTRAINT,
            null
        };

        SqliteAlterTable();
        SqliteAlterTable(const SqliteAlterTable& other);
        ~SqliteAlterTable();

        void initRenameTable(const QString& name1, const QString& name2, const QString& newName);
        void initAddColumn(const QString& name1, const QString& name2, bool columnKw, SqliteCreateTable::Column* column);
        void initDropColumn(const QString& name1, const QString& name2, bool columnKw, const QString& dropColumn);
        void initRenameColumn(const QString& name1, const QString& name2, bool columnKw, const QString& oldColumnName, const QString& newColumnName);
        void initColumnSetNotNull(const QString& name1, const QString& name2, bool columnKw, const QString& colName, SqliteConflictAlgo algo);
        void initColumnDropNotNull(const QString& name1, const QString& name2, bool columnKw, const QString& colName);
        void initAddCheck(const QString& name1, const QString& name2, const QString& constrName, SqliteExpr* checkExpr, SqliteConflictAlgo algo);
        void initAddCheck(const QString& name1, const QString& name2, SqliteExpr* checkExpr, SqliteConflictAlgo algo);
        void initDropConstraint(const QString& name1, const QString& name2, const QString& constrName);
        SqliteStatement* clone();

    protected:
        QStringList getColumnsInStatement();
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getColumnTokensInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents(bool replaceStatementTokens) const;

    private:
        void initName(const QString& name1, const QString& name2);

    public:
        Command command = Command::null;
        QString newName = QString();
        QString database = QString();
        QString table = QString();
        QString dropColumnName = QString();
        QString columnName = QString();
        QString newColumnName = QString();
        QString constraintName = QString();
        SqliteExpr* expr = nullptr;
        bool columnKw = false;
        SqliteCreateTable::Column* newColumn = nullptr;
        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
};

typedef QSharedPointer<SqliteAlterTable> SqliteAlterTablePtr;

#endif // SQLITEALTERTABLE_H
