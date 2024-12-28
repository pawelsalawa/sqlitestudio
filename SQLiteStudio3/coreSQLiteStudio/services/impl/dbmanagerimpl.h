#ifndef DBMANAGERIMPL_H
#define DBMANAGERIMPL_H

#include "db/db.h"
#include "coreSQLiteStudio_global.h"
#include "common/strhash.h"
#include "common/global.h"
#include "services/dbmanager.h"
#include <QObject>
#include <QList>
#include <QHash>
#include <QReadWriteLock>
#include <QSharedPointer>

class InvalidDb;

class API_EXPORT DbManagerImpl : public DbManager
{
    Q_OBJECT

    public:
        /**
         * @brief Creates database manager.
         * @param parent Parent object passed to QObject constructor.
         */
        explicit DbManagerImpl(QObject *parent = 0);

        /**
         * @brief Default destructor.
         */
        ~DbManagerImpl();

        bool addDb(const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent = true);
        bool addDb(const QString &name, const QString &path, bool permanent = true);
        bool updateDb(Db* db, const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent);
        void removeDbByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive);
        void removeDbByPath(const QString& path);
        void removeDb(Db* db);
        QList<Db*> getDbList();
        QList<Db*> getValidDbList();
        QList<Db*> getConnectedDbList();
        QStringList getDbNames();
        QStringList getValidDbNames();
        Db* getByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseInsensitive);
        Db* getByPath(const QString& path);
        Db* createInMemDb(bool pureInit = false);
        bool isTemporary(Db* db);
        QString quickAddDb(const QString &path, const QHash<QString, QVariant> &options);
        DbPlugin* getPluginForDbFile(const QString& filePath);
        QString generateUniqueDbName(const QString& filePath);
        QString generateUniqueDbName(DbPlugin* plugin, const QString& filePath);

        /**
         * @brief Defines database plugin used for creating in-memory databases.
         * @param plugin Plugin to use.
         */
        void setInMemDbCreatorPlugin(DbPlugin* plugin);

    private:
        /**
         * @brief Internal manager initialization.
         *
         * Called from any constructor.
         */
        void init();

        /**
         * @brief Loads initial list of databases.
         *
         * Loaded databases are initially the invalid databases.
         * They are turned into valid databases once their plugins are loaded.
         */
        void loadInitialDbList();

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
         * @brief Filters invalid databases from all managed databases.
         * @return Only invalid databases from this manager.
         */
        QList<Db*> getInvalidDatabases() const;

        Db* tryToLoadDb(InvalidDb* invalidDb, bool emitNotifySignal = true);

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
        static Db* createDb(const QString &name, const QString &path, const QHash<QString, QVariant> &options, QString* errorMessages = nullptr);

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

        /**
         * @brief Database plugin used to create in-memory databases.
         */
        DbPlugin* inMemDbCreatorPlugin = nullptr;

        QList<DbPlugin*> dbPlugins;

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
         * @brief Passes Db::aboutToDisconnect() signal to dbAboutToBeDisconnected() signal.
         */
        void dbAboutToDisconnect(bool& deny);

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
        void notifyDatabasesAreLoaded();
        void scanForNewDatabasesInConfig();
        void rescanInvalidDatabasesForPlugin(DbPlugin* dbPlugin);
};

#endif // DBMANAGERIMPL_H
