#ifndef TABLEMODIFIER_H
#define TABLEMODIFIER_H

#include "db/db.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"

class API_EXPORT TableModifier
{
    public:
        TableModifier(Db* db, const QString& table);
        TableModifier(Db* db, const QString& database, const QString& table);

        void alterTable(SqliteCreateTablePtr newCreateTable);

        QStringList generateSqls() const;
        bool isValid() const;
        QStringList getErrors() const;
        QStringList getWarnings() const;
        QStringList getModifiedTables() const;
        QStringList getModifiedIndexes() const;
        QStringList getModifiedTriggers() const;
        QStringList getModifiedViews() const;
        bool hasMessages() const;

    private:
        void init();
        void parseDdl();
        QString getTempTableName() const;
        void copyDataTo(const QString& targetTable, const QStringList& srcCols, const QStringList& dstCols);
        void renameTo(const QString& newName);
        QString renameToTemp();
        void copyDataTo(const QString& table);
        void copyDataTo(SqliteCreateTablePtr newCreateTable);

        void handleIndexes();
        void handleIndex(SqliteCreateIndexPtr index);
        void handleTriggers();
        void handleTrigger(SqliteCreateTriggerPtr trigger);
        void handleViews();
        void handleView(SqliteCreateViewPtr view);
        SqliteQuery* handleTriggerQuery(SqliteQuery* query, const QString& trigName);
        SqliteSelect* handleSelect(SqliteSelect* select);
        SqliteUpdate* handleTriggerUpdate(SqliteUpdate* update, const QString& trigName);
        SqliteInsert* handleTriggerInsert(SqliteInsert* insert, const QString& trigName);
        SqliteDelete* handleTriggerDelete(SqliteDelete* del, const QString& trigName);
        bool handleSubSelects(SqliteStatement* stmt);
        bool handleExprWithSelect(SqliteExpr* expr);
        void simpleHandleIndexes();
        void simpleHandleTriggers(const QString& view = QString::null);
        SqliteQueryPtr parseQuery(const QString& ddl);

        /**
         * @brief alterTableHandleFks
         * @param newCreateTable
         * Finds all tables referencing currently modified table and updates their referenced table name and columns.
         */
        void handleFks(const QString& tempTableName);
        void subHandleFks(const QString& oldName, const QString& oldTempName);
        bool subHandleFks(SqliteForeignKey* fk, const QString& oldName, const QString& oldTempName);

        bool handleName(const QString& oldName, QString& valueToUpdate);
        bool handleIndexedColumns(QList<SqliteIndexedColumn*>& columnsToUpdate);
        bool handleColumnNames(QStringList& columnsToUpdate);
        bool handleColumnTokens(TokenList& columnsToUpdate);
        bool handleUpdateColumns(SqliteUpdate* update);

        Db* db = nullptr;
        Dialect dialect;

        /**
         * @brief database Database name. The "main" is default.
         * Other databases (temp, attached...) are not supported at the moment.
         */
        QString database;

        /**
         * @brief table Current table name (after renaming)
         */
        QString table;

        /**
         * @brief originalTable Initial table name, before any renaming.
         */
        QString originalTable;

        /**
         * @brief createTable Original DDL.
         */
        SqliteCreateTablePtr createTable;

        /**
         * @brief sqls Statements to be executed to make changes real.
         */
        QStringList sqls;

        QStringList warnings;
        QStringList errors;

        QString newName;
        QStringList existingColumns;
        QHash<QString, QString> tableColMap;
        QStringList modifiedTables;
        QStringList modifiedIndexes;
        QStringList modifiedTriggers;
        QStringList modifiedViews;
};

#endif // TABLEMODIFIER_H
