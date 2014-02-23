#ifndef QUERYEXECUTORCELLSIZE_H
#define QUERYEXECUTORCELLSIZE_H

#include "queryexecutorstep.h"

/**
 * @brief Applies per-cell byte size limit to the query.
 *
 * Size of data extracted for each cell is limited in order to avoid huge memory use
 * when the database contains column with like 500MB values per row and the query
 * returns for example 100 rows.
 *
 * This is accomplished by wrapping all result columns (except ROWID columns) with substr() SQL function.
 *
 * SQLiteStudio limits each column to SqlQueryModel::cellDataLengthLimit when displaying
 * data in SqlQueryView.
 *
 * This feature is disabled by default in QueryExecutor and has to be enabled by defining
 * QueryExecutor::setDataLengthLimit().
 */
class QueryExecutorCellSize : public QueryExecutorStep
{
    Q_OBJECT

    public:
        bool exec();

    private:
        /**
         * @brief Applies limit function to all result columns in given SELECT.
         * @param select Select that we want to limit.
         * @param core Select's core that we want to limit.
         * @return true on success, false on failure.
         *
         * This method is called for each core in the \p select.
         */
        bool applyDataLimit(SqliteSelect* select, SqliteSelect::Core* core);

        /**
         * @brief Generates tokens that will return limited value of the result column.
         * @param resCol Result column to wrap.
         * @return List of tokens.
         */
        TokenList getLimitTokens(const QueryExecutor::ResultColumnPtr& resCol);

        /**
         * @brief Generates tokens that will return unlimited value of the ROWID result column.
         * @param resCol ROWID result column.
         * @return List of tokens.
         */
        TokenList getNoLimitTokens(const QueryExecutor::ResultRowIdColumnPtr& resCol);

        /**
         * @brief Generates tokens representing result columns separator.
         * @return List of tokens.
         *
         * Result columns separator tokens are just a period and a space.
         */
        TokenList getSeparatorTokens();

        /**
         * @brief Puts the SELECT as a subselect.
         * @param selectTokens All tokens of the original SELECT.
         * @param resultColumnsTokens Result columns tokens for the new SELECT.
         * @return New SELECT tokens.
         *
         * Original SELECT tokens are put into subselect of the new SELECT statement. New SELECT statement
         * is built using given \p resultColumnTokens.
         */
        TokenList wrapSelect(const TokenList& selectTokens, const TokenList& resultColumnsTokens);
};

#endif // QUERYEXECUTORCELLSIZE_H
