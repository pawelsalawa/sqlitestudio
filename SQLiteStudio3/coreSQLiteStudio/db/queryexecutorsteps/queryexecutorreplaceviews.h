#ifndef QUERYEXECUTORREPLACEVIEWS_H
#define QUERYEXECUTORREPLACEVIEWS_H

#include "queryexecutorstep.h"
#include "parser/ast/sqlitecreateview.h"

class SchemaResolver;

/**
 * @brief Replaces all references to views in query with SELECTs from those views.
 *
 * Replacing views with their SELECTs (as subselects) simplifies later tasks
 * with the query.
 */
class QueryExecutorReplaceViews : public QueryExecutorStep
{
        Q_OBJECT

    public:
        ~QueryExecutorReplaceViews();

        bool exec();

    protected:
        void init();

    private:
        /**
         * @brief View representation in context of QueryExecutorReplaceViews step.
         */
        struct View
        {
            /**
             * @brief Creates view.
             * @param database Database of the view.
             * @param view View name.
             */
            View(const QString& database, const QString& view);

            /**
             * @brief Database of the view.
             */
            QString database;

            /**
             * @brief View name.
             */
            QString view;

            /**
             * @brief Checks if it's the same view as the \p other.
             * @param other Other view to compare.
             * @return 1 if other view is the same one, or 0 otherwise.
             *
             * Views are equal if they have equal name and database.
             */
            int operator==(const View& other) const;
        };

        friend TYPE_OF_QHASH qHash(const View& view);

        /**
         * @brief Provides all views existing in the database.
         * @param database Database name as typed in the query.
         * @return List of view names.
         *
         * Uses internal cache (using views).
         */
        QStringList getViews(const QString& database);

        /**
         * @brief Reads view's DDL, parses it and returns results.
         * @param database Database of the view.
         * @param viewName View name.
         * @return Parsed view or null pointer if view doesn't exist or could not be parsed.
         *
         * It uses internal cache (using viewStatements).
         */
        SqliteCreateViewPtr getView(const QString& database, const QString& viewName);

        /**
         * @brief Replaces views in the query with SELECT statements.
         * @param select SELECT statement to replace views in.
         *
         * It explores the \p select looking for view names and replaces them with
         * apropriate subselect queries, using getView() calls.
         */
        void replaceViews(SqliteSelect* select);

        /**
         * @brief Tells whether particular SELECT statement has any View as a data source.
         * @param select Parsed SELECT statement.
         * @param viewsInDatabase Prepared list of views existing in the database.
         * @return true if the SELECT uses at least one existing View.
         */
        bool usesAnyView(SqliteSelect* select, const QStringList& viewsInDatabase);

        /**
         * @brief Used for caching view list per database.
         */
        QHash<QString,QStringList> views;

        /**
         * @brief Resolver used several time in this step.
         *
         * It's stored as member of this class, cause otherwise it would be created
         * and deleted many times. Instead it's shared across all calls to resolve something
         * from schema.
         */
        SchemaResolver* schemaResolver = nullptr;

        /**
         * @brief Used for caching parsed view statement.
         */
        QHash<View,SqliteCreateViewPtr> viewStatements;
};

TYPE_OF_QHASH qHash(const QueryExecutorReplaceViews::View& view);

#endif // QUERYEXECUTORREPLACEVIEWS_H
