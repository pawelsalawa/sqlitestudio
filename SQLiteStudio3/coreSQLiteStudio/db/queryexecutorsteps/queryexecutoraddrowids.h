#ifndef QUERYEXECUTORADDROWIDS_H
#define QUERYEXECUTORADDROWIDS_H

#include "queryexecutorstep.h"
#include "selectresolver.h"

/**
 * @brief Adds ROWID to result columns.
 *
 * This step adds ROWID to result column list for each table mentioned in result columns.
 * For WITHOUT ROWID tables there might be several columns per table.
 *
 * It also provides list of added columns in QueryExecutor::Context::rowIdColumns.
 */
class QueryExecutorAddRowIds : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();

    private:
        /**
         * @brief Adds ROWID columns to the result columns and the context.
         * @param core SELECT's core that keeps result columns.
         * @param table Table we want to add ROWID columns for.
         * @param rowIdColsMap Map of ROWID columns from inner selects (built with addRowIdForTables()).
         * @param isTopSelect True only for top-most select to store rowid columns in context only for the final list of columns.
         * @return true on success, false on any failure.
         *
         * Finds columns representing ROWID for the \p table and adds them to result columns and to the context.
         */
        bool addResultColumns(SqliteSelect::Core* core, const SelectResolver::Table& table,
                              QHash<SelectResolver::Table, QHash<QString, QString> >& rowIdColsMap, bool isTopSelect);

        /**
         * @brief Adds the column to result columns list.
         * @param core SELECT's core that keeps the result columns.
         * @param table Table that the column is for.
         * @param queryExecutorColumn Alias name for the column that will be used by the query executor.
         * @param realColumn Actual column name in the database.
         * @return true on success, false on any failure.
         *
         * Adds given column to the result column list in the SELECT statement.
         */
        bool addResultColumns(SqliteSelect::Core* core, const SelectResolver::Table& table, const QString& queryExecutorColumn,
                              const QString& realColumn, bool aliasOnlyAsSelectColumn);

        /**
         * @brief Adds all necessary ROWID columns to result columns.
         * @param select SELECT that keeps result columns.
         * @param ok[out] Reference to a flag for telling if the method was executed successly (true), or not (false).
         * @param isTopSelect True only for top-most select call of this method, so the list of rowid columns is stored
         * only basing on this select (and rowid mappind for it), not all subqueries. This is to avoid redundant rowid columns in context
         * in case of subselects.
         * @return Mapping for every table mentioned in the SELECT with map of ROWID columns for the table.
         * The column map is a query_executor_alias to real_database_column_name.
         *
         * Adds ROWID columns for all tables mentioned in result columns of the \p select.
         */
        QHash<SelectResolver::Table,QHash<QString,QString>> addRowIdForTables(SqliteSelect* select, bool& ok, bool isTopSelect = true);

        /**
         * @brief Extracts all subselects used in the SELECT.
         * @param core SELECT's core to extract subselects from.
         * @return List of subselects.
         *
         * Extracts only subselects of given select core, but not recurrently.
         * As it works on the SELECT's core, it means that it's not applicable for compound selects.
         */
        QList<SqliteSelect*> getSubSelects(SqliteSelect::Core* core);

        /**
         * @brief Provides list of columns representing ROWID for the table.
         * @param table Table to get ROWID columns for.
         * @return Map of query executor alias to real database column name.
         */
        QHash<QString, QString> getNextColNames(const SelectResolver::Table& table);

        bool checkInWithClause(const SelectResolver::Table& table, SqliteWith *with);
};

#endif // QUERYEXECUTORADDROWIDS_H
