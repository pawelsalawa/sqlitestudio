#ifndef COMPLETIONCOMPARER_H
#define COMPLETIONCOMPARER_H

#include "expectedtoken.h"
#include "selectresolver.h"

class CompletionHelper;

class CompletionComparer
{
    public:
        explicit CompletionComparer(CompletionHelper* helper);

        bool operator()(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);

    private:
        CompletionHelper* helper = nullptr;
        /**
         * @brief contextDatabases
         * Context objects are any names mentioned anywhere in the query at the same level as completion takes place.
         * The level means the sub-query level, for example in case of sub selects.
         */
        QStringList contextDatabases;
        QStringList contextTables;
        QStringList contextColumns;

        /**
         * @brief parentContextDatabases
         * Parent context objects are any names mentioned anywhere in the the query at all upper levels.
         */
        QStringList parentContextDatabases;
        QStringList parentContextTables;
        QStringList parentContextColumns;
        QList<SelectResolver::Column> resultColumns;

        /**
         * @brief availableTableNames
         * Names of all tables mentioned in FROM clause in the current and all parent select cores.
         */
        QStringList availableTableNames;

        void init();
        bool initSelect();
        bool compareColumns(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);
        bool compareColumnsForSelectResCol(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result);
        bool compareColumnsForUpdateCol(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result);
        bool compareColumnsForDeleteCol(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result);
        bool compareColumnsForInsertCol(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result);
        bool compareColumnsForCreateTable(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result);
        bool compareColumnsForReturning(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result);
        bool compareTables(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);
        bool compareIndexes(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);
        bool compareTriggers(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);
        bool compareViews(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);
        bool compareDatabases(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2);
        bool compareValues(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool handleSystemNames = false);
        bool compareValues(const QString& token1, const QString& token2, bool handleSystemNames = false);
        bool compareByContext(const QString &token1, const QString &token2,
                              const QStringList& contextValues, bool* ok = nullptr);
        bool compareByContext(const QString &token1, const QString &token2,
                              const QList<QStringList>& contextValues, bool* ok = nullptr);
        bool compareByContext(const QString &token1, const QString &token2,
                              const QStringList& contextValues, bool handleSystemNames, bool* ok = nullptr);
        bool compareByContext(const QString &token1, const QString &token2,
                              const QList<QStringList>& contextValues, bool handleSystemNames, bool* ok = nullptr);
        bool compareByContextOnly(const QString &token1, const QString &token2,
                              const QStringList& contextValues, bool handleSystemNames, bool* ok);
        bool isTokenOnAvailableColumnList(const ExpectedTokenPtr& token);
        bool isTokenOnParentAvailableColumnList(const ExpectedTokenPtr& token);
        bool isTokenOnResultColumns(const ExpectedTokenPtr& token);
        static bool isTokenOnColumnList(const ExpectedTokenPtr& token, const QList<SelectResolver::Column>& columnList);
};

#endif // COMPLETIONCOMPARER_H
