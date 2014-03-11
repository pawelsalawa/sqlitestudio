#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "db.h"
#include "coreSQLiteStudio_global.h"
#include "strhash.h"
#include "global.h"
#include <QObject>
#include <QList>
#include <QHash>
#include <QReadWriteLock>
#include <QSharedPointer>

/** @file */

class DbPlugin;
class Config;
class Plugin;
class PluginType;

/**
 * @brief Database registry manager.
 *
 * Manages list of databases in SQLiteStudio core.
 * Also keeps list of supported database types
 * (QSqlDriver names), like "QSQLITE", etc.
 *
 * It's a singleton asseccible with DBLIST macro.
 */
class API_EXPORT DbManager : public QObject
{
    Q_OBJECT

    DECLARE_SINGLETON(DbManager)

    public:
        /**
         * @brief Adds database to the manager.
         * @param name Symbolic name of the database, as it will be presented in the application.
         * @param path Path to the database file.
         * @param options Key-value custom options for database, that can be used in the DbPlugin implementation, like connection password, etc.
         * @param permanent If true, then the database will be remembered in configuration, otherwise it will be disappear after application restart.
         * @return true if the database has been successfly added, or false otherwise.
         *
         * The method can return false if given database file exists, but is not supported SQLite version (including invalid files,
         * that are not SQLite database). It basicly returns false if DbPlugin#getInstance() returned null for given database parameters.
         */
        bool addDb(const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent = true);

        /**
         * @overload bool addDb(const QString &name, const QString &path, bool permanent)
         */
        bool addDb(const QString &name, const QString &path, bool permanent = true);

        /**
         * @brief Updates registered database with new data.
         * @param db Registered database.
         * @param name New symbolic name for the database.
         * @param path New database file path.
         * @param options New database options. See addDb() for details.
         * @param permanent True to make the database stored in configuration, false to make it disappear after application restart.
         * @return true if the database was successfly updated, or false otherwise.
         */
        bool updateDb(Db* db, const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent);

        /**
         * @brief Removes database from application.
         * @param name Symbolic name of the database.
         * @param cs Should the name be compare with case sensitivity?
         */
        void removeDbByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive);

        /**
         * @brief Removes database from application.
         * @param path Database file path as it was passed to addDb() or updateDb().
         */
        void removeDbByPath(const QString& path);

        /**
         * @brief Removes database from application.
         * @param db Database to be removed.
         */
        void removeDb(Db* db);

        /**
         * @brief Gives list of databases registered in the application.
         * @return List of databases, no matter if database is open or not.
         */
        QList<Db*> getDbList();

        /**
         * @brief Gives list of currently open databases.
         * @return List of open databases.
         */
        QList<Db*> getConnectedDbList();

        /**
         * @brief Gives list of database names.
         * @return List of database names that are registered in the application.
         */
        QStringList getDbNames();

        /**
         * @brief Gives database object by its name.
         * @param name Symbolic name of the database.
         * @param cs Should the \p name be compared with case sensitivity?
         * @return Database object, or null pointer if the database could not be found.
         *
         * This method is fast, as it uses hash table lookup.
         */
        Db* getByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive);

        /**
         * @brief Gives database object by its file path.
         * @param path Database file path as it was passed to addDb() or updateDb().
         * @return Database matched by file path, or null if no database was found.
         *
         * This method is fast, as it uses hash table lookup.
         */
        Db* getByPath(const QString& path);

        /**
         * @brief Generates database name.
         * @param filePath Database file path.
         * @return A name, using database file name as a hint for a name.
         *
         * This method doesn't care about uniqueness of the name. It just gets the file name from provided path
         * and uses it as a name.
         */
        static QString generateDbName(const QString& filePath);

    private:
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
         * @brief Internal manager initialization.
         *
         * Called from any constructor.
         */
        void init();

        /**
         * @brief Removes database from application.
         * @param db Database to be removed.
         * @param alsoFromConfig If true, database will also be removed from configuration file, otherwise it's just from the manager.
         *
         * This method is internally called by public methods, as they all do pretty much the same thing,
         * except they accept different input parameter. Then this method does the actual job.
         */
        void removeDbInternal(Db* db, bool alsoFromConfig = true);

        /**
         * @brief Adds database to the application.
         * @param db Database to be added.
         * @param alsoToConfig If true, the database will also be added to configuration file, otherwise it will be onle to the manager.
         *
         * When addDb() is called, it calls DbPlugin#getInstance() and if it returns object, then this method
         * is called to register the database object in dbList variable.
         */
        void addDbInternal(Db* db, bool alsoToConfig = true);

        /**
         * @brief Creates database object.
         * @param name Symbolic name of the database.
         * @param path Database file path.
         * @param options Database options, such as password, etc.
         * @param errorMessages If not null, then the error messages from DbPlugins are stored in that string (in case this method returns null).
         * @return Database object, or null pointer.
         *
         * This method is used internally by addDb() methods. It goes through all DbPlugin instances
         * and checks if any of them supports given file path and options and returns a database object.
         * First plugin that provides database object is accepted and its result is returned from the method.
         */
        Db* createDbObj(const QString &name, const QString &path, const QHash<QString, QVariant> &options, QString* errorMessages = nullptr);

        /**
         * @brief Registered databases list. Both permanent and transient databases.
         */
        QList<Db*> dbList;

        /**
         * @brief Database ame to database instance mapping, with keys being case insensitive.
         */
        StrHash<Db*> nameToDb;

        /**
         * @brief Mapping from file path to the database.
         *
         * Mapping from database file path (as passed to addDb() or updateDb()) to the actual database object.
         */
        QHash<QString,Db*> pathToDb;

        /**
         * @brief Lock for dbList.
         * Lock for dbList, so the list can be accessed from multiple threads.
         */
        QReadWriteLock listLock;

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
         * @param plugin Plugin that handles the database.
         *
         * Emitted after a plugin was loaded and it turned out to handle the database that was already registered in the application,
         * but wasn't managed by database manager, because no handler plugin was loaded.
         */
        void dbLoaded(Db* db, DbPlugin* plugin);

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
         * @brief Emited when the initial database list has been loaded.
         */
        void dbListLoaded();

    private slots:
        /**
         * @brief Slot called when connected to db.
         *
         * The slot is connected to the database object, therefore the database object has to be extracted from signal sender
         * and converted to database type, then passed to the dbConnected(Db* db) signal.
         */
        void dbConnectedSlot();
        /**
         * @brief Slot called when connected to db.
         *
         * The slot is connected to the database object, therefore the database object has to be extracted from signal sender
         * and converted to database type, then passed to the dbConnected(Db* db) signal.
         */
        void dbDisconnectedSlot();

        /**
         * @brief Removes databases handled by the plugin from the list.
         * @param plugin DbPlugin (any other will be ignored).
         * @param type DbPlugin type.
         * It removes all databases handled by the plugin being unloaded from the list of managed databases.
         */
        void aboutToUnload(Plugin* plugin, PluginType* type);

        /**
         * @brief Adds all configured databases handled by the plugin to managed list.
         * @param plugin DbPlugin (any other will be ignored).
         * @param type DbPlugin type.
         * Checks configuration for any databases managed by the plugin and if there is any, it's loaded into the managed list.
         */
        void loaded(Plugin* plugin, PluginType* type);

    public slots:
        /**
         * @brief Tries to load all databases from configuration.
         *
         * Gets list of registered databases from configuration and for each of them
         * tries to find working DbPlugin. After the registered database was loaded successfully
         * by some DbPlugin, the database gets registered in DbManager and will later be
         * provided by getDbList(), getByName() and getByPath() methods.
         *
         * Any databases that failed to be loaded are not registered in DbManager.
         * To get full list of registered databases (even those not loaded), use Config::dbList().
         */
        void loadDbListFromConfig();
};

/**
 * @brief Database manager.
 * Provides direct access to database manager.
 */
#define DBLIST DbManager::getInstance()

#endif // DBMANAGER_H
