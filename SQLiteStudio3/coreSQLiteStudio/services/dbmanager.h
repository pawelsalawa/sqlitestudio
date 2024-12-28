#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "db/db.h"
#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <QList>
#include <QHash>

/** @file */

class DbPlugin;
class Config;
class Plugin;
class PluginType;

/**
 * @brief Database registry manager.
 *
 * Manages list of databases in SQLiteStudio core.
 *
 * It's a singleton asseccible with DBLIST macro.
 */
class API_EXPORT DbManager : public QObject
{
    Q_OBJECT

    public:
        /**
         * @brief Creates database manager.
         * @param parent Parent object passed to QObject constructor.
         */
        explicit DbManager(QObject *parent = 0);

        /**
         * @brief Default destructor.
         */
        ~DbManager();

        /**
         * @brief Adds database to the manager.
         * @param name Symbolic name of the database, as it will be presented in the application.
         * @param path Path to the database file.
         * @param options Key-value custom options for database, that can be used in the DbPlugin implementation, like connection password, etc.
         * @param permanent If true, then the database will be remembered in configuration, otherwise it will be disappear after application restart.
         * @return true if the database has been successfully added, or false otherwise.
         *
         * The method can return false if given database file exists, but is not supported SQLite version (including invalid files,
         * that are not SQLite database). It basicly returns false if DbPlugin#getInstance() returned null for given database parameters.
         */
        virtual bool addDb(const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent = true) = 0;

        /**
         * @overload
         */
        virtual bool addDb(const QString &name, const QString &path, bool permanent = true) = 0;

        /**
         * @brief Adds database as temporary, with generated name.
         * @param path Path to database.
         * @param options Key-value custom options for database.
         * @return Added database name, if the database has been successfully added, or null string otherwise.
         *
         * This method is used for example when database was passed as argument to application command line arguments.
         */
        virtual QString quickAddDb(const QString &path, const QHash<QString, QVariant> &options) = 0;

        /**
         * @brief Updates registered database with new data.
         * @param db Registered database.
         * @param name New symbolic name for the database.
         * @param path New database file path.
         * @param options New database options. See addDb() for details.
         * @param permanent True to make the database stored in configuration, false to make it disappear after application restart.
         * @return true if the database was successfully updated, or false otherwise.
         */
        virtual bool updateDb(Db* db, const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent) = 0;

        /**
         * @brief Removes database from application.
         * @param name Symbolic name of the database.
         * @param cs Should the name be compare with case sensitivity?
         */
        virtual void removeDbByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive) = 0;

        /**
         * @brief Removes database from application.
         * @param path Database file path as it was passed to addDb() or updateDb().
         */
        virtual void removeDbByPath(const QString& path) = 0;

        /**
         * @brief Removes database from application.
         * @param db Database to be removed.
         */
        virtual void removeDb(Db* db) = 0;

        /**
         * @brief Gives list of databases registered in the application.
         * @return List of databases, no matter if database is open or not.
         *
         * The results list includes invalid databases (not supported by driver plugin, or with no read access, etc).
         */
        virtual QList<Db*> getDbList() = 0;

        /**
         * @brief Gives list of valid databases.
         * @return List of valid databases.
         */
        virtual QList<Db*> getValidDbList() = 0;

        /**
         * @brief Gives list of currently open databases.
         * @return List of open databases.
         */
        virtual QList<Db*> getConnectedDbList() = 0;

        /**
         * @brief Gives list of database names.
         * @return List of database names that are registered in the application.
         */
        virtual QStringList getDbNames() = 0;

        /**
         * @brief Gives list of valid database names.
         * @return List of database names that are registered and valid (no errors) in the application.
         */
        virtual QStringList getValidDbNames() = 0;

        /**
         * @brief Gives database object by its name.
         * @param name Symbolic name of the database.
         * @param cs Should the \p name be compared with case sensitivity?
         * @return Database object, or null pointer if the database could not be found.
         *
         * This method is fast, as it uses hash table lookup.
         */
        virtual Db* getByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseInsensitive) = 0;

        /**
         * @brief Gives database object by its file path.
         * @param path Database file path as it was passed to addDb() or updateDb().
         * @return Database matched by file path, or null if no database was found.
         *
         * This method is fast, as it uses hash table lookup.
         */
        virtual Db* getByPath(const QString& path) = 0;

        /**
         * @brief Creates in-memory SQLite3 database.
         * @param pureInit If true, avoids registering collations/functions/extensions in a database. Skips rich initialization and gives pure database connection.
         * @return Created database.
         *
         * Created database can be used for any purpose. Note that DbManager doesn't own created
         * database and it's up to the caller to delete the database when it's no longer needed.
         */
        virtual Db* createInMemDb(bool pureInit = false) = 0;

        /**
         * @brief Tells if given database is temporary.
         * @param db Database to check.
         * @return true if database is temporary, or false if it's stored in the configuration.
         *
         * Temporary databases are databases that are not stored in configuration and will not be restored
         * upon next SQLiteStudio start. This can be decided by user on UI when he edits database registration info
         * (there is a checkbox for that).
         */
        virtual bool isTemporary(Db* db) = 0;

        virtual DbPlugin* getPluginForDbFile(const QString& filePath) = 0;
        virtual QString generateUniqueDbName(const QString& filePath) = 0;
        virtual QString generateUniqueDbName(DbPlugin* plugin, const QString& filePath) = 0;

        /**
         * @brief Generates database name.
         * @param filePath Database file path.
         * @return A name, using database file name as a hint for a name.
         *
         * This method doesn't care about uniqueness of the name. It just gets the file name from provided path
         * and uses it as a name.
         */
        static QString generateDbName(const QString& filePath);

    public slots:
        /**
         * @brief Rescans configuration for new database entries.
         *
         * Looks into the configuration for new databases. If there are any, adds them to list of managed databases.
         */
        virtual void scanForNewDatabasesInConfig() = 0;

        /**
         * @brief Sends signal to all interested entities, that databases are loaded.
         *
         * This is called by the managing entity (the SQLiteStudio instance) to let all know,
         * that all db-related plugins and configuration related to databases are now loaded
         * and list of databases in the manager is complete.
         */
        virtual void notifyDatabasesAreLoaded() = 0;

        virtual void rescanInvalidDatabasesForPlugin(DbPlugin* dbPlugin) = 0;

    signals:
        /**
         * @brief Application just connected to the database.
         * @param db Database object that the connection was made to.
         *
         * Emitted just after application has connected to the database.
         */
        void dbConnected(Db* db);

        /**
         * @brief Application just disconnected from the database.
         * @param db Database object that the connection was closed with.
         */
        void dbDisconnected(Db* db);

        /**
         * @brief The database is about to be disconnected and user can still deny it.
         * @param db Database to be closed.
         * @param deny If set to true, then disconnecting will be aborted.
         */
        void dbAboutToBeDisconnected(Db* db, bool& deny);

        /**
         * @brief A database has been added to the application.
         * @param db Database added.
         * Emitted from addDb() methods in case of success.
         */
        void dbAdded(Db* db);

        /**
         * @brief A database has been removed from the application.
         * @param db Database object that was removed. The object still exists, but will be removed soon after this signal is handled.
         *
         * Emitted from removeDb(). As the argument is a smart pointer, the object will be deleted after last reference to the pointer
         * is deleted, which is very likely that the pointer instance in this signal is the last one.
         */
        void dbRemoved(Db* db);

        /**
         * @brief A database registration data has been updated.
         * @param oldName The name of the database before the update - in case the name was updated.
         * @param db Database object that was updated.
         *
         * Emitted from updateDb() after successful update.
         *
         * The name of the database is a key for tables related to the databases, so if it changed, we dbUpdated() provides
         * the original name before update, so any tables can be updated basing on the old name.
         */
        void dbUpdated(const QString& oldName, Db* db);

        /**
         * @brief Loaded plugin to support the database.
         * @param db Database object handled by the plugin.
         *
         * Emitted after a plugin was loaded and it turned out to handle the database that was already registered in the application,
         * but wasn't managed by database manager, because no handler plugin was loaded earlier.
         *
         * Also emitted when database details were edited and saved, which fixes database configuration (for example path).
         */
        void dbLoaded(Db* db);

        /**
         * @brief Plugin supporting the database is about to be unloaded.
         * @param db Database object to be removed from the manager.
         * @param plugin Plugin that handles the database.
         *
         * Emitted when PluginManager is about to unload the plugin which is handling the database.
         * All classes using this database object should stop using it immediately, or the application may crash.
         *
         * The plugin itself should not use this signal. Instead it should implement Plugin::deinit() method
         * to perform deinitialization before unloading. The Plugin::deinit() method is called before this signal is emitted.
         */
        void dbAboutToBeUnloaded(Db* db, DbPlugin* plugin);

        /**
         * @brief Plugins supporting the database was just unloaded.
         * @param db The new database object (InvalidDb) that replaced the previous one.
         *
         * This is emitted after the plugin for the database was unloaded. The \p db object is now a different object.
         * It is of InvalidDb class and it represents a database in an invalid state. It still has name, path and connection options,
         * but no operation can be performed on the database.
         */
        void dbUnloaded(Db* db);

        /**
         * @brief Emited when the initial database list has been loaded.
         */
        void dbListLoaded();
};

/**
 * @brief Database manager.
 * Provides direct access to the database manager.
 */
#define DBLIST SQLITESTUDIO->getDbManager()

#endif // DBMANAGER_H
