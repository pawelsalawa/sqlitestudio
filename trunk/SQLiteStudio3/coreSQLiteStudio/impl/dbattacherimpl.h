#ifndef DBATTACHERIMPL_H
#define DBATTACHERIMPL_H

#include "dbattacher.h"

class DbAttacherImpl : public DbAttacher
{
    public:
        /**
         * @brief Creates attacher with the given database as the main.
         * @param db Database that the query will be executed on.
         */
        DbAttacherImpl(Db* db);

        bool attachDatabases(const QString& query);
        bool attachDatabases(const QList<SqliteQueryPtr>& queries);
        bool attachDatabases(SqliteQueryPtr query);
        void detachDatabases();
        BiStrHash getDbNameToAttach() const;
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

        QList<SqliteQueryPtr> queries;
        Db* db = nullptr;
        Dialect dialect;
        BiStrHash dbNameToAttach;
        StrHash<Db*> nameToDbMap;
};

class DbAttacherDefaultFactory : public DbAttacherFactory
{
    public:
        DbAttacher* create(Db* db);
};

#endif // DBATTACHERIMPL_H
