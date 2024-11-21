#ifndef TABLEMODIFIER_H
#define TABLEMODIFIER_H

#include "db/db.h"
#include "selectresolver.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "common/strhash.h"

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
        QString getTempTableName();
        void copyDataTo(const QString& targetTable, const QStringList& srcCols, const QStringList& dstCols);
        void renameTo(const QString& newName, bool doCopyData = true);
        QString renameToTemp(bool doCopyData = true);
        void copyDataTo(const QString& table);
        void copyDataTo(SqliteCreateTablePtr newCreateTable);

        void handleIndexes();
        void handleIndex(SqliteCreateIndexPtr index);
        void handleTriggers();
        void handleTrigger(SqliteCreateTriggerPtr trigger);
        void handleTriggerQueries(SqliteCreateTriggerPtr trigger);
        void handleViews();
        void handleView(SqliteCreateViewPtr view);
        SqliteQuery* handleTriggerQuery(SqliteQuery* query, const QString& trigName, const QString& trigTable);
        SqliteSelect* handleSelect(SqliteSelect* select, const QString& trigTable = QString());
        SqliteUpdate* handleTriggerUpdate(SqliteUpdate* update, const QString& trigName, const QString& trigTable);
        SqliteInsert* handleTriggerInsert(SqliteInsert* insert, const QString& trigName, const QString& trigTable);
        SqliteDelete* handleTriggerDelete(SqliteDelete* del, const QString& trigName, const QString& trigTable);
        StrHash<SelectResolver::Table> tablesAsNameHash(const QSet<SelectResolver::Table> &resolvedTables);
        bool isTableAliasUsedForColumn(const TokenPtr& token, const StrHash<SelectResolver::Table>& resolvedTables, const QList<SqliteSelect::Core::SingleSource*>& selSources);
        bool handleSubSelects(SqliteStatement* stmt, const QString& trigTable);
        bool handleExprWithSelect(SqliteExpr* expr, const QString& trigTable);
        bool handleAllExprWithTrigTable(SqliteStatement* stmt, const QString& contextTable);
        bool handleExprListWithTrigTable(const QList<SqliteExpr*>& exprList);
        bool handleExprWithTrigTable(SqliteExpr* expr);
        bool handleExpr(SqliteExpr* expr);
        void simpleHandleIndexes();
        void simpleHandleTriggers(const QString& view = QString());
        SqliteQueryPtr parseQuery(const QString& ddl);

        /**
         * @brief alterTableHandleFks
         * @param newCreateTable
         * Finds all tables referencing currently modified table and updates their referenced table name and columns.
         */
        void handleFks();
        void handleFkAsSubModifier(const QString& oldName, const QString& theNewName);
        bool handleFkStmt(SqliteForeignKey* fk, const QString& oldName, const QString& theNewName);
        bool handleFkConstrains(SqliteCreateTable* stmt, const QString& oldName, const QString& theNewName);

        bool handleName(const QString& oldName, QString& valueToUpdate);
        static bool handleName(const QString& oldName, const QString& theNewName, QString& valueToUpdate);
        bool handleIndexedColumns(const QList<SqliteOrderBy*>& columnsToUpdate);
        bool handleIndexedColumnsInitial(SqliteOrderBy* col, bool& modified);
        bool handleIndexedColumnsInitial(SqliteIndexedColumn* col, bool& modified);
        bool handleColumnNames(QStringList& columnsToUpdate);
        bool handleColumnTokens(TokenList& columnsToUpdate);
        bool handleUpdateColumns(SqliteUpdate* update);
        QStringList handleUpdateColumns(const QStringList& colNames, bool& modified);
        QString handleUpdateColumn(const QString& colName, bool& modified);
        QList<SqliteCreateTable::Column*> getColumnsToCopyData(SqliteCreateTablePtr newCreateTable);

        template <class T>
        bool handleIndexedColumns(QList<T*>& columnsToUpdate)
        {
            bool modified = false;
            QString lowerName;
            QString colName;
            QMutableListIterator<T*> it(columnsToUpdate);
            while (it.hasNext())
            {
                T* idxCol = it.next();
                if (handleIndexedColumnsInitial(idxCol, modified))
                    continue;

                colName = idxCol->getColumnName();

                // If column was modified, assign new name
                lowerName = colName.toLower();
                if (tableColMap.contains(lowerName))
                {
                    idxCol->setColumnName(tableColMap[lowerName]);
                    modified = true;
                    continue;
                }

                // It wasn't modified, but it's not on existing columns list? Remove it.
                if (indexOf(existingColumns, colName, Qt::CaseInsensitive) == -1)
                {
                    it.remove();
                    modified = true;
                }
            }
            return modified;
        }

        Db* db = nullptr;

        /**
         * @brief Database name. The "main" is default.
         * Other databases (temp, attached...) are not supported at the moment.
         */
        QString database;

        /**
         * @brief Current table name (after renaming)
         */
        QString table;

        /**
         * @brief Initial table name, before any renaming.
         */
        QString originalTable;

        /**
         * @brief createTable Original DDL.
         */
        SqliteCreateTablePtr createTable;

        /**
         * @brief Statements to be executed to make changes real.
         */
        QStringList sqls;

        QStringList warnings;
        QStringList errors;

        QString newName;
        QStringList existingColumns;
        QHash<QString, QString> tableColMap;
        QHash<QString, QString> triggerNameToDdlMap;
        QStringList tablesHandledForFk;
        QStringList modifiedTables;
        QStringList modifiedIndexes;
        QStringList modifiedTriggers;
        QStringList modifiedViews;
        QStringList usedTempTableNames;
};


#endif // TABLEMODIFIER_H
