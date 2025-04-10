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
class API_EXPORT DbAttacher
{
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~DbAttacher();

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
        virtual bool attachDatabases(const QString& query) = 0;

        /**
         * Be aware that database names in queries are replaced with attach names in SqliteStatement::tokens,
         * thus modified query can be achived with SqliteStatement::detokenize(). This also means
         * that the input queries will contain modified token list.
         *
         * @overload
         */
        virtual bool attachDatabases(const QList<SqliteQueryPtr>& queries) = 0;

        /**
         * @overload
         */
        virtual bool attachDatabases(SqliteQueryPtr query) = 0;

        /**
         * @brief Detaches all databases attached by the attacher.
         */
        virtual void detachDatabases() = 0;

        /**
         * @brief Provides mapping of database names to their attach names.
         * @return Database name to attach name mapping.
         *
         * The returned map is bi-directional, so you can easly translate database name to attach name
         * and vice versa. Left values of the map are database names (as registered in DbManager)
         * and right values are attach names assigned to them.
         */
        virtual BiStrHash getDbNameToAttach() const = 0;

        /**
         * @brief Provides query string updated with attach names.
         * @return Query string.
         */
        virtual QString getQuery() const = 0;

        /**
         * @brief Tells if "main" database name was used in the query and was ommited.
         * @return true when "main" db token was in the query, or false otherwise.
         *
         * The "main" database will not be attached and this getter tells wheter such situation occurred.
         * This is to assert situation when user has database on the list which name is actually "main".
         * Table window always uses "main." prefix when reading data and this caused attacher to step in,
         * which then caused some execution error.
         */
        virtual bool getMainDbNameUsed() const = 0;
};

/**
 * @brief Abstract factory for DbAttacher objects.
 *
 * The abstract factory is accessed from SQLiteStudio class in order to produce DbAttacher instances.
 * The default DbAttacherFactory implementation (DbAttacherFactoryImpl) produces default DbAttacher instances
 * (DbAttacherImpl), but it can be replaced with other factory to produce other attachers, just like unit tests
 * in this project do.
 */
class API_EXPORT DbAttacherFactory
{
    public:
        virtual ~DbAttacherFactory();

        /**
         * @brief Produces single attacher instance.
         * @param db Database to produce attacher for.
         * @return Attacher instance. Factory doesn't own it, you have to delete it when you're done.
         */
        virtual DbAttacher* create(Db* db) = 0;
};

#endif // DBATTACHER_H
