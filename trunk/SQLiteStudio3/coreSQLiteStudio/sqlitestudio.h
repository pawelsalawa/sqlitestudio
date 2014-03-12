#ifndef SQLITESTUDIO_H
#define SQLITESTUDIO_H

#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include <QString>
#include <QStringList>
#include <QObject>

class DbManager;
class Config;
class QProcessEnvironment;
class PluginManager;
class QThreadPool;
class NotifyManager;
class SqlFormatter;
class Plugin;
class PluginType;
class FunctionManager;

/** @file */

/**
 * @mainpage
 * SQLiteStudio is SQLite 2 and 3 manager for Windows, MacOS X and Linux.
 *
 * Global variables and macros:
 * <ul>
 * <li>#SQLITESTUDIO - access point to all services (singleton instance of SQLiteStudio)</li>
 * <li>#PLUGINS - quick access to PluginManager</li>
 * <li>#DBLIST - quick access to DbManager</li>
 * </ul>
 */


/**
 * @brief Main application class.
 * This is an entry point for all services.
 * Get all managers, services, etc from this class.
 * It is a singleton.
 */
class API_EXPORT SQLiteStudio : public QObject
{
        Q_OBJECT

        DECLARE_SINGLETON(SQLiteStudio)

    public:
        /**
         * @brief Gets current SQL formatter instance.
         * @return Currently configured SQL formatter instance, or first available formatter (if it's not yet configured), or null if there is no formatter available.
         */
        SqlFormatter* getSqlFormatter() const;

        /**
         * @brief Initializes SQLiteStudio object.
         * @param cmdListArguments Command line arguments.
         *
         * Initialization process involves creating of all internal objects (managers, etc.)
         * and reading necessary configuration. It also interpreted command line arguments
         * and applies them.
         * See parseCmdLineArgs() for details on supported options.
         */
        void init(const QStringList& cmdListArguments);

        /**
         * @brief Gets environment variable value.
         * @param name Name of the environment variable.
         * @param defaultValue Default value to be returned if the environment variable is not defined.
         * @return Either value of the environment variable - if defined - or the value passed as defaultValue.
         *
         * This method provides cross-platform way to get environment variable value.
         * Internally it uses QProcessEnvironment, but while it's expensive to initialize it every time you access environment,
         * it keeps the single instance of that object and lets you query variables by name.
         */
        QString getEnv(const QString& name, const QString& defaultValue = QString());

    private:
        /**
         * @brief Creates singleton instance.
         *
         * It doesn't initialize anything, just constructs object.
         * Initialization of member data is done by init() method.
         */
        SQLiteStudio();

        /**
         * @brief Deinitializes object.
         *
         * Calls cleanUp().
         */
        ~SQLiteStudio();

        /**
         * @brief Cleans up all internal objects.
         *
         * Deletes all internal objects. It's called from destructor.
         */
        void cleanUp();

        /**
         * @brief Parses command line arguments.
         *
         * It parses and applies command line arguments.
         * TODO: describe options
         */
        void parseCmdLineArgs();

        /**
         * @brief Current SQL formatter instance.
         *
         * SQL formatter is created from SqlFormatterPlugin.
         * Current sqlFormatter value is picked from collection of formatters
         * delivered by plugins by configuration entry.
         * If it's not configured, then first available formatter is chosen.
         * If no SQL formatter plugins are available, then this variable
         * remains null.
         */
        SqlFormatter* sqlFormatter = nullptr;

        /**
         * @brief The application environment.
         *
         * This variable represents environment of the application.
         * It provides access to environment variables.
         */
        QProcessEnvironment* env = nullptr;

        /**
         * @brief List of command line arguments.
         *
         * It's a copy of arguments passed to application in command line.
         */
        QStringList cmdLineArgs;

    private slots:
        void pluginLoaded(Plugin* plugin,PluginType* pluginType);
        void pluginToBeUnloaded(Plugin* plugin,PluginType* pluginType);
        void pluginUnloaded(const QString& pluginName,PluginType* pluginType);

    public slots:
        /**
         * @brief Updates active SQL formatter.
         *
         * Reads active formatter name from configuration and picks that formatter from list of loaded plugins
         * to be used as active formatter. If there's no formatter configured, or the one configured is not on the list
         * of loaded plugins, then no formatter is set to active and getSqlFormatter() will return SQL strings unchanged.
         */
        void updateSqlFormatter();
};

/**
 * @def SQLITESTUDIO
 * @brief Global entry point for application services.
 *
 * This macro actually calls SQLiteStudio::getInstance(), which returns singleton instance
 * of the main class, which is SQLiteStudio. Use this class as starting point
 * to access all services of the application (database manager, plugins manager, etc).
 * This singleton instance is created at the very begining of application start (in main())
 * and so can be used from pretty much everywhere in the code.
 *
 * Quick example of getting all databases registered in the application, iterating through them and printing
 * their name to standard output:
 * @code
   #include "qio.h"
   #include "sqlitestudio.h"

   void someFunction()
   {
       QList<Db*> dblist = SQLITESTUDIO->getDbManager()->getDbList();
       foreach (Db* db, dblist)
       {
           qOut << db->getName();
       }
   }
   @endcode
 */
#define SQLITESTUDIO SQLiteStudio::getInstance()

#endif // SQLITESTUDIO_H
