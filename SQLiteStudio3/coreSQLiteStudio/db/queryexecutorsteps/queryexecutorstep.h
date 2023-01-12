#ifndef QUERYEXECUTORSTEP_H
#define QUERYEXECUTORSTEP_H

#include "parser/ast/sqliteselect.h"
#include "db/queryexecutor.h"
#include <QObject>

/**
 * @brief Base class for all query executor steps.
 *
 * Query Executor step is a single step that query executor performs at a time
 * during the "smart execution" (see QueryExecutor for details).
 *
 * Steps are created and queued in QueryExecutor::setupExecutionChain().
 * Order of steps execution is very important and should not be distrurbed,
 * unless you know what you're doing.
 *
 * A step must implement one method: exec().
 * The implementation can access QueryExecutor instance that is running the step
 * and also it can access common context for executor and all the steps.
 *
 * Steps usually introduce some modifications to the query, so the final execution
 * can provide more meta-information, or works on limited set of data, etc.
 *
 * Steps can access common context to get parsed object of the current query.
 * The current query is the query processed by previous steps and re-parsed after
 * those modifications. Current query is also available in a string representation
 * in the context. The original query string (before any modifications) is also
 * available in the context. See QueryExecutor::Context for more.
 *
 * QueryExecutorStep provides several methods to help dealing with common routines,
 * such as updating current query with modified query definition (updateQueries()),
 * or extracting parsed SELECT (if the last query defined was the SELECT) object
 * (getSelect()). When the step needs to add new result column, it can use
 * getNextColName() to generate unique name.
 *
 * To access database object, that the query is executed on, use QueryExecutor::getDb().
 */
class API_EXPORT QueryExecutorStep : public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Cleans up resources.
         */
        virtual ~QueryExecutorStep();

        /**
         * @brief Initializes step with a pointer to calling executor and a common context.
         * @param queryExecutor Calling query executor.
         * @param context Common context, shared among query executor and all steps.
         *
         * This method also calls virtual method init(), which can be used to one-time setup
         * in derived step.
         */
        void init(QueryExecutor* queryExecutor, QueryExecutor::Context* context);

        /**
         * @brief Executes step routines.
         * @return result of execution - successful or failed.
         *
         * The exec() method should execute all necessary routines required by this step.
         * If it needs to execute anything on the database
         *
         * Once the false is returned by exec(), there should be no signal emitted by the step.
         */
        virtual bool exec() = 0;

    protected:
        /**
         * @brief Updates QueryExecutor::Context::processedQuery string.
         *
         * Goes through all queries in QueryExecutor::Context::parsedQueries and detokenizes
         * their tokens, concatenatins all results into QueryExecutor::Context::processedQuery.
         *
         * This should be called every time tokens of any parsed query were modified
         * and you want those changes to be reflected in the processed query.
         *
         * See QueryExecutor::Context::processedQuery for more details;
         */
        void updateQueries();

        /**
         * @brief Generates unique name for result column alias.
         * @return Unique name.
         *
         * When a step needs to add another column to results, it can use this method
         * to make sure that the name (alias) of this column will be unique across
         * other result columns.
         *
         * QueryExecutorColumn step makes sure that every result column processed
         * by query executor gets its own unique alias name.
         *
         * The getNextColName() whould be used only once per result column. When forwarding
         * the result column from inner select to outer select, use the same alias name
         * ad in the inner select.
         */
        QString getNextColName();

        /**
         * @brief Extracts SELECT object from parsed queries.
         * @return Parsed SELECT, or null pointer.
         *
         * This is a helper method. Since most of the logic in steps is required in regards
         * of the last SELECT statement in all queries, this method comes handy.
         *
         * If the last statement in executed queries is the SELECT, then this method
         * returns parsed object of that statement, otherwise it returns null pointer.
         */
        SqliteSelectPtr getSelect();

        /**
         * @brief Custom initialization of the step.
         *
         * If a step needs some initial code to be executed, or some members to be initialized,
         * this is the place to put it into.
         */
        virtual void init();

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

        /**
         * @brief Pointer to the calling executor.
         */
        QueryExecutor* queryExecutor = nullptr;

        /**
         * @brief Pointer to a shared context for all steps.
         */
        QueryExecutor::Context* context = nullptr;

        /**
         * @brief Database that all queries will be executed on.
         *
         * Defined by init().
         */
        Db* db = nullptr;
};

#endif // QUERYEXECUTORSTEP_H
