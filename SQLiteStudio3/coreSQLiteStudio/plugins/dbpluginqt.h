#ifndef DBPLUGINQT_H
#define DBPLUGINQT_H

#include "dbplugin.h"
#include "plugins/genericplugin.h"

class DbQt;

/**
 * @brief DbPlugin class based on Qt's database framework.
 *
 * Use this class to implement database plugins that can rely on QtSql module.
 * It implements most of the DbPlugin interface, leaving just few simple routines
 * to be implemented in derived class.
 */
class API_EXPORT DbPluginQt : public DbPlugin, public GenericPlugin
{
    public:
        /**
         * @brief Creates database plugin.
         */
        DbPluginQt();
        ~DbPluginQt();

        Db* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage);
        QString generateDbName(const QVariant& baseValue);
        QList<DbPluginOption> getOptionsList() const;
        bool isRemote() const;
        bool init();
        void deinit();

    protected:
        /**
         * @brief Creates new instance of the database implemented by this plugin.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         * @return Instance of the database.
         */
        virtual DbQt* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options) = 0;

        /**
         * @brief Provides driver name valid for QSqlDatabase::addDatabase().
         * @return Driver name.
         *
         * Standard Qt driver names for SQLite are: QSQLITE and QSQLITE2.
         */
        virtual QString getDriver() = 0;

    private:
        /**
         * @brief Tests if given database file is valid for this plugin implementation.
         * @param path Database file.
         * @param options Connection options (passed from getInstance()).
         * @param errorMessage If the result is false and this pointer is not null, the error message will be stored in it.
         * @return true if the database file is valid, or false otherwise.
         *
         * This method opens given file and executes "<tt>SELECT * FROM sqlite_master;</tt>" query.
         * If that's successful, it returns true. Otherwise it returns false.
         */
        bool probe(const QString& path, const QHash<QString, QVariant> &options, QString* errorMessage);

        /**
         * @brief Defines QSqlDatabase for probing databases.
         *
         * It registers its own QSqlDatabase just for probing purpose.
         * The database is deregistered in constructor.
         * Driver for probing is gained from getDriver().
         */
        void initProbeConnection();

        /**
         * @brief Name of the probing QSqlDatabase.
         *
         * It's defined in initProbeConnection() and is later used
         * in destructor to deregister it.
         */
        QString probeConnName;
};

#endif // DBPLUGINQT_H
