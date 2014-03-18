#ifndef DBPLUGIN_H
#define DBPLUGIN_H

#include "db/db.h"
#include "db/dbpluginoption.h"
#include "plugins/plugin.h"

/**
 * @brief Interface for database plugins
 *
 * This is a specialization of Plugin interface, which all database plugins have to implement.
 */
class API_EXPORT DbPlugin : virtual public Plugin
{
    public:
        /**
         * @brief Creates database instance defined by the plugin.
         * @param name Name for the database.
         * @param path Path to the database file.
         * @param options Options for the database passed while registering the database in the application.
         * @param errorMessage If the result is null (on failure) and this pointer is not null, the error message will be stored in it.
         * @return Database instance on success, or null pointer on failure.
         *
         * Options can contain for example password for an encrypted database, or other connection options.
         */
        virtual Db* getInstance(const QString& name, const QString& path, const QHash<QString,QVariant> &options, QString* errorMessage = 0) = 0;

        /**
         * @brief Provides label of what type is the database.
         * @return Type label.
         *
         * The label is used for presenting to the user what kind of database this is. It's used on GUI
         * to display database type in databases dialog. It's usually either "SQLite3" or "SQLite2",
         * but it may be something else, like for example encrypted database might provide "Encrypted SQLite3",
         * or something similar.
         */
        virtual QString getLabel() const = 0;

        /**
         * @brief Provides list of options configurable for this database plugin.
         * @return List of options.
         *
         * DbDialog uses this to provide GUI interface, so user can configure connection options.
         * For each element in the list DbDialog adds QLabel and the input widget for entering option value.
         * Option values entered by user are later passed to getInstance() as second argument.
         */
        virtual QList<DbPluginOption> getOptionsList() const = 0;

        /**
         * @brief Generates suggestion for database name.
         * @param baseValue Value enterd as file path in DbDialog.
         * @return Generated name suggestion.
         *
         * This can be simply string representation of \p baseValue,
         * but the plugin may add something characteristic for the plugin.
         */
        virtual QString generateDbName(const QVariant& baseValue) = 0;

        /**
         * @brief Tests if the given database support is provided by this plugin.
         * @param db Database to test.
         * @return true if the database is supported by this plugin, or false otherwise.
         *
         * Implementation of this method should check if given database object
         * is of the same type, that those returned from getInstance().
         *
         * This method is used by DbManager to find out which databases are supported by which plugins,
         * so when some plugin is about to be unloaded, all its databases are closed properly first.
         */
        virtual bool checkIfDbServedByPlugin(Db* db) const = 0;
};

#endif // DBPLUGIN_H
