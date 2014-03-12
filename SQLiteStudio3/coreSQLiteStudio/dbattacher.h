#ifndef DBATTACHER_H
#define DBATTACHER_H

#include "parser/ast/sqlitequery.h"
#include "common/bistrhash.h"
#include "common/strhash.h"
#include <QList>

class Db;

/**
 * @brief Transparent attaching of databases used in the query.
 *
 * Scans query (or queries) for names of databases registered in DbManager.
 * If a database is recognized, it's automatically attached and the mapping
 * of its database name to its attach name is created for later information.
 *
 * The attacher looks for database names only at those spots in the query,
 * where the database name is valid by SQLite syntax (this is accomplished
 * with SqliteStatement::getContextDatabaseTokens()).
 */
class DbAttacher
{
    public:
        /**
         * @brief Creates attacher with the given database as the main.
         * @param db Database that the query will be executed on.
         */
        DbAttacher(Db* db);

        /**
         * @brief Scans for databases in given query and attaches them.
         * @param query Query string to be executed.
         * @return true on success, or false on failure.
         *
         * The method can fail if any of databases used in the query could
         * not be attached (the ATTACH statement caused error).
         *
         * To get query with database names replaced with attach names use getQuery().
         */
        bool attachDatabases(const QString& query);

        /**
         * Be aware that database names in queries are replaced with attach names in SqliteStatement::tokens,
         * thus modified query can be achived with SqliteStatement::detokenize(). This also means
         * that the input queries will contain modified token list.
         *
         * @overload bool attachDatabases(const QList<SqliteQueryPtr>& queries)
         */
        bool attachDatabases(const QList<SqliteQueryPtr>& queries);

        /**
         * @overload bool attachDatabases(SqliteQueryPtr query)
         */
        bool attachDatabases(SqliteQueryPtr query);

        /**
         * @brief Detaches all databases attached by the attacher.
         */
        void detachDatabases();

        /**
         * @brief Provides mapping of database names to their attach names.
         * @return Database name to attach name mapping.
         *
         * The returned map is bi-directional, so you can easly translate database name to attach name
         * and vice versa. Left values of the map are database names (as registered in DbManager)
         * and right values are attach names assigned to them.
         */
        BiStrHash getDbNameToAttach() const;

        /**
         * @brief Provides query string updated with attach names.
         * @return Query string.
         */
        QString getQuery() const;

    private:
        /**
         * @brief Does the actual job, after having all input queries as parsed objects.
         * @return true on success, false on failure.
         */
        bool attachDatabases();

        /**
         * @brief Finds tokens representing databases in the query.
         * @return List of tokens. Some tokens have non-printable value (spaces, etc), others are database names.
         */
        TokenList getDbTokens();

        /**
         * @brief Detaches all databases currently attached by the attacher.
         *
         * Also clears names mappings.
         */
        void detachAttached();

        /**
         * @brief Generates mapping of database name to its Db object for all registered databases.
         */
        void prepareNameToDbMap();

        /**
         * @brief Groups tokens by the name of database they refer to.
         * @param dbTokens Tokens representing databases in the query.
         *
         * This method is used to learn if some database is used more than once in the query,
         * so we attach it only once, then replace all tokens referring to it by the attach name.
         */
        QHash<QString,TokenList> groupDbTokens(const TokenList& dbTokens);

        /**
         * @brief Tries to attach all required databases.
         * @param groupedDbTokens Database tokens grouped by database name, as returned from groupDbTokens().
         * @return true on success, false on any problem.
         *
         * Major problem that can happen is when "<tt>ATTACH 'path to file'</tt>" fails for any reason. In that case
         * detachAttached() is called and false is returned.
         */
        bool attachAllDbs(const QHash<QString,TokenList>& groupedDbTokens);

        /**
         * @brief Creates token-to-token replace map to update the query.
         * @param dbTokens Tokens representing databases in the query.
         * @return Mapping to be used when replacing tokens in the query.
         */
        QHash<TokenPtr, TokenPtr> getTokenMapping(const TokenList& dbTokens);

        /**
         * @brief Replaces tokens in the query.
         * @param tokenMapping Map of tokens to replace.
         *
         * Replacing takes place in token lists of each query in the queries member.
         */
        void replaceTokensInQueries(const QHash<TokenPtr, TokenPtr>& tokenMapping);

    private:
        QList<SqliteQueryPtr> queries;
        Db* db = nullptr;
        Dialect dialect;
        BiStrHash dbNameToAttach;
        StrHash<Db*> nameToDbMap;
};

#endif // DBATTACHER_H
