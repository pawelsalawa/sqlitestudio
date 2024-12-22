#ifndef QUERYEXECUTORCOLUMNS_H
#define QUERYEXECUTORCOLUMNS_H

#include "queryexecutorstep.h"
#include "selectresolver.h"

/**
 * @brief Assigns unique alias names for all result columns.
 *
 * This step replaces result columns of the SELECT query.
 * It's performed only if last query is the SELECT, otherwise it does nothing.
 *
 * It works on subselects first, then goes towards outer SELECTs.
 *
 * Star operator ("all columns") is replaced by list of columns and each column gets alias.
 *
 * If result column comes from subselect and the subselect was already covered by this step,
 * then the column does not get new alias, instead the existing one is used.
 *
 * While generating alias names, this step also finds out details about columns: source database, source table
 * column contraints, etc. Those informations are stored using generated alias name as a key.
 *
 * Some columns can be defined as not editable, because of various reasons: QueryExecutor::ColumnEditionForbiddenReason.
 * Those reasons are defined in this step.
 */
class QueryExecutorColumns : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();

    private:

        /**
         * @brief Transforms SelectResolver's columns into QueryExecutor's columns.
         * @param resolvedColumn Result columns resolved by SelectResolver.
         * @return Converted column.
         *
         * QueryExecutor understands different model of result columns than SelectResolver.
         * Converted columns are later used by other steps and it's also returned from QueryExecutor as an information
         * about result columns of the query.
         */
        QueryExecutor::ResultColumnPtr getResultColumn(const SelectResolver::Column& resolvedColumn);

        /**
         * @brief Generates result column object with proper alias name.
         * @param resultColumn Original result column from the query.
         * @param col Original result column as resolved by SelectResolver.
         * @param rowIdColumn Indicates if this is a call for ROWID column added by QueryExecutorRowId step.
         * @return Result column object ready for rebuilding tokens and detokenizing.
         */
        SqliteSelect::Core::ResultColumn* getResultColumnForSelect(const QueryExecutor::ResultColumnPtr& resultColumn, const SelectResolver::Column& col, QSet<QString>& usedAliases);

        /**
         * @brief Translates attach name into database name.
         * @param dbName Attach name.
         * @return Database name as registered in DbManager, or \p dbName if given name was not resolved to any registered database.
         */
        QString resolveAttachedDatabases(const QString& dbName);

        /**
         * @brief Checks if given alias name belongs to ROWID result column.
         * @param alias Alias name to check.
         * @return true if the alias belongs to ROWID column, or false otherwise.
         */
        bool isRowIdColumnAlias(const QString& alias);

        bool isRowIdColumn(const QString& columnAlias);
        QStringList rowIdColNames;
};

#endif // QUERYEXECUTORCOLUMNS_H
