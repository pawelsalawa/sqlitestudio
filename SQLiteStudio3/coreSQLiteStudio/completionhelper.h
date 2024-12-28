#ifndef COMPLETIONHELPER_H
#define COMPLETIONHELPER_H

#include "expectedtoken.h"
#include "schemaresolver.h"
#include "selectresolver.h"
#include "completioncomparer.h"
#include "parser/ast/sqliteselect.h"
#include "parser/token.h"
#include "db/db.h"
#include "common/strhash.h"
#include "common/table.h"
#include <QObject>
#include <QSet>

class DbAttacher;

class API_EXPORT CompletionHelper : public QObject
{
        Q_OBJECT

    friend class CompletionComparer;

    public:
        struct API_EXPORT Results
        {
            QList<ExpectedTokenPtr> filtered();

            QList<ExpectedTokenPtr> expectedTokens;
            QString partialToken;
            bool wrappedToken = false;
        };

        CompletionHelper(const QString& sql, Db* db);
        CompletionHelper(const QString& sql, quint32 cursorPos, Db* db);
        ~CompletionHelper();

        static void applyFilter(QList<ExpectedTokenPtr> &results, const QString &filter);
        static void init();

        Results getExpectedTokens();

        DbAttacher* getDbAttacher() const;
        void setDbAttacher(DbAttacher* value);

        static bool enableLemonDebug;

        QString getCreateTriggerTable() const;
        void setCreateTriggerTable(const QString& value);

    private:
        enum class Context
        {
            NONE,
            SELECT_RESULT_COLUMN,
            SELECT_FROM,
            SELECT_WHERE,
            SELECT_GROUP_BY,
            SELECT_HAVING,
            SELECT_ORDER_BY,
            SELECT_LIMIT,
            UPDATE_COLUMN,
            UPDATE_WHERE,
            DELETE_WHERE,
            CREATE_TABLE,
            CREATE_TRIGGER,
            EXPR,
            INSERT_COLUMNS,
            INSERT_RETURNING,
            UPDATE_RETURNING,
            DELETE_RETURNING
        };

        static void initFunctions(Db* db);
        static void initPragmas(Db* db);

        QList<ExpectedTokenPtr> getExpectedTokens(TokenPtr token);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString& value);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString& value,
                                          int priority);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString& value,
                                          const QString& contextInfo);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString& value,
                                          const QString& contextInfo, int priority);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString &value,
                                          const QString &contextInfo, const QString &label);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString &value,
                                          const QString &contextInfo, const QString &label,
                                          int priority);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString &value,
                                          const QString &contextInfo, const QString &label,
                                          const QString &prefix);
        ExpectedTokenPtr getExpectedToken(ExpectedToken::Type type, const QString &value,
                                          const QString &contextInfo, const QString &label,
                                          const QString &prefix, int priority);
        bool validatePreviousIdForGetObjects(QString* dbName = nullptr);
        QList<ExpectedTokenPtr> getTables();
        QList<ExpectedTokenPtr> getIndexes();
        QList<ExpectedTokenPtr> getTriggers();
        QList<ExpectedTokenPtr> getViews();
        QList<ExpectedTokenPtr> getDatabases();
        QList<ExpectedTokenPtr> getObjects(ExpectedToken::Type type);
        QList<ExpectedTokenPtr> getObjects(ExpectedToken::Type type, const QString& database);
        QList<ExpectedTokenPtr> getColumns();
        QList<ExpectedTokenPtr> getColumnsNoPrefix();
        QList<ExpectedTokenPtr> getColumnsNoPrefix(const QString &column, const QStringList &tables);
        QList<ExpectedTokenPtr> getColumns(const QString& prefixTable);
        QList<ExpectedTokenPtr> getColumns(const QString& prefixDb, const QString& prefixTable);
        QList<ExpectedTokenPtr> getFavoredColumns(const QList<ExpectedTokenPtr>& resultsSoFar);

        QList<ExpectedTokenPtr> getPragmas();
        QList<ExpectedTokenPtr> getFunctions(Db* db);
        QList<ExpectedTokenPtr> getCollations();
        TokenPtr getPreviousDbOrTable(const TokenList& parsedTokens);
        void attachDatabases();
        void detachDatabases();
        QString translateDatabase(const QString& dbName);
        QString translateDatabaseBack(const QString& dbName);
        void collectOtherDatabases();
        QString removeStartedToken(const QString& adjustedSql, QString &finalFilter, bool& wrappedFilter);
        void filterContextKeywords(QList<ExpectedTokenPtr> &results, const TokenList& tokens);
        void filterOtherId(QList<ExpectedTokenPtr> &results, const TokenList& tokens);
        void filterDuplicates(QList<ExpectedTokenPtr> &results);
        bool isFilterType(Token::Type type);
        void parseFullSql();
        bool tryToParse(Parser* parser, const QString& query);
        void sort(QList<ExpectedTokenPtr> &results);
        void extractPreviousIdTokens(const TokenList& parsedTokens);
        void extractQueryAdditionalInfo();
        void extractSelectAvailableColumnsAndTables();
        void extractInsertAvailableColumnsAndTables();
        void extractDeleteAvailableColumnsAndTables();
        void extractUpdateAvailableColumnsAndTables();
        void extractAvailableColumnsAndTables(const QString& database, const QString& table);
        bool extractSelectCore();
        SqliteSelect::Core* extractSelectCore(SqliteQueryPtr query);
        void extractTableAliasMap();
        void extractCreateTableColumns();
        void detectSelectContext();
        bool isInUpdateColumn();
        bool isInUpdateWhere();
        bool isInDeleteWhere();
        bool isInCreateTable();
        bool isInCreateTrigger();
        bool isInInsertColumns();
        bool isInUpdateReturning();
        bool isInDeleteReturning();
        bool isInInsertReturning();
        bool isIn(SqliteQueryType queryType, const QString& tokenMapKey, const QString &prefixKeyword);
        bool isInExpr();
        bool testQueryToken(int tokenPosition, Token::Type type, const QString& value, Qt::CaseSensitivity cs = Qt::CaseInsensitive);
        bool cursorAfterTokenMaps(SqliteStatement* stmt, const QStringList& mapNames);
        bool cursorBeforeTokenMaps(SqliteStatement* stmt, const QStringList& mapNames);

        template <class T>
        bool cursorFitsInCollection(const QList<T*>& collection)
        {
            if (collection.size() == 0)
                return false;

            T* firstStmt = collection.first();
            T* lastStmt = collection.last();

            int startIdx = -1;
            int endIdx = -1;
            if (firstStmt && firstStmt->tokens.size() > 0)
                startIdx = firstStmt->tokens.first()->start;

            if (lastStmt && lastStmt->tokens.size() > 0)
                endIdx = lastStmt->tokens.last()->end;

            if (startIdx < 0 || endIdx < 0)
                return false;

            return (cursorPosition >= startIdx && cursorPosition <= endIdx);
        }

        template <class T>
        bool cursorFitsInStatement(T* stmt)
        {
            if (!stmt || stmt->tokens.size() == 0)
                return false;

            int startIdx = stmt->tokens.first()->start;
            int endIdx = stmt->tokens.last()->end;

            return (cursorPosition >= startIdx && cursorPosition <= endIdx);
        }


        Context context = Context::NONE;
        Db* db = nullptr;
        qint64 cursorPosition;
        QString fullSql;
        TokenPtr previousId;
        TokenPtr twoIdsBack;
        TokenList queryTokens;
        SqliteQueryPtr parsedQuery;
        SqliteQueryPtr originalParsedQuery;
        SchemaResolver* schemaResolver = nullptr;
        SelectResolver* selectResolver = nullptr;
        DbAttacher* dbAttacher = nullptr;
        QString createTriggerTable;

        /**
         * @brief tableToAlias
         * This map maps real table name to its alias. Every table can be typed multiple times
         * with different aliases, therefore this map has a list on the right side.
         */
        QHash<QString,QStringList> tableToAlias;

        /**
         * @brief aliasToTable
         * Maps table alias to table's real name.
         */
        //QHash<QString,QString> aliasToTable;
        StrHash<Table> aliasToTable;

        /**
         * @brief currentSelectCore
         * The SqliteSelect::Core object that contains current cursor position.
         */
        SqliteSelect::Core* currentSelectCore = nullptr;

        /**
         * @brief originalCurrentSelectCore
         *
         * The same as currentSelectCore, but relates to originalParsedQuery, not just parsedQuery.
         */
        SqliteSelect::Core* originalCurrentSelectCore = nullptr;

        /**
         * @brief selectAvailableColumns
         * Available columns are columns that can be selected basing on what tables are mentioned in FROM clause.
         */
        QList<SelectResolver::Column> selectAvailableColumns;

        /**
         * @brief selectAvailableTables
         * Availble tables are tables mentioned in FROM clause.
         */
        QSet<SelectResolver::Table> selectAvailableTables;

        /**
         * @brief theFromAvailableColumns
         * Available columns are columns that can be selected basing on what tables are mentioned in FROM clause
         * of queries other than SELECT (so INSERT, UPDATE, DELETE), in their RETURNING clause.
         */
        QList<SelectResolver::Column> theFromAvailableColumns;

        /**
         * @brief theFromAvailableTables
         * Availble tables are tables mentioned in FROM clause of queries other than SELECT (so INSERT, UPDATE, DELETE),
         * in their RETURNING clause.
         */
        QSet<SelectResolver::Table> theFromAvailableTables;

        /**
         * @brief parentSelectCores
         * List of all parent select core objects in order: from direct parent, to the oldest parent.
         */
        QList<SqliteSelect::Core*> parentSelectCores;

        /**
         * @brief parentSelectAvailableColumns
         * List of all columns available in all tables mentioned in all parent select cores.
         */
        QList<SelectResolver::Column> parentSelectAvailableColumns;

        /**
         * @brief parentSelectAvailableTables
         * Same as parentSelectAvailableColumns, but for tables in FROM clauses.
         */
        QSet<SelectResolver::Table> parentSelectAvailableTables;

        /**
         * @brief favoredColumnNames
         * For some contexts there are favored column names, like for example CREATE TABLE
         * will favour column names specified so far in its definition.
         * Such columns will be added to completion results even they weren't result
         * of regular computation.
         */
        QStringList favoredColumnNames;

        /**
         * @brief Context databases to look for objects in.
         *
         * If the query uses some other databases (not the currentone), then user might be interested
         * also in objects from those databases. This list contains those databases.
         */
        QStringList otherDatabasesToLookupFor;
};

#endif // COMPLETIONHELPER_H
