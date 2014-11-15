#ifndef SQLITESTUDIO_H
#define SQLITESTUDIO_H

#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include "services/config.h"
#include <QString>
#include <QStringList>
#include <QObject>

class DbManager;
class Config;
class QProcessEnvironment;
class PluginManager;
class QThreadPool;
class NotifyManager;
class CodeFormatter;
class Plugin;
class PluginType;
class FunctionManager;
class DbAttacherFactory;
class DbAttacher;
class ExportManager;
class ImportManager;
class PopulateManager;
class PluginLoadingHandler;
class BugReporter;
class UpdateManager;
class ExtraLicenseManager;

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
 * <li>#FUNCTIONS - quick access to FunctionManager</li>
 * <li>#CFG - quick access to Config</li>
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
         * @brief Initializes SQLiteStudio object.
         * @param cmdListArguments Command line arguments.
         * @param pluginLoadingHandler Factory for producing plugin loader.
         *
         * Initialization process involves creating of all internal objects (managers, etc.)
         * and reading necessary configuration. It also interpreted command line arguments
         * and applies them.
         *
         * The plugin loader factory (handler) is used to solve issue with GUI symbols visibility. while loading code being placed in the core shared library.
         * It should be null when starting SQLiteStudio in CLI mode and not null when starting GUI client. See PluginLoadingHandler for more details on that.
         *
         * See parseCmdLineArgs() for details on supported options.
         */
        void init(const QStringList& cmdListArguments, bool guiAvailable);

        void initPlugins();

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

        /**
         * @brief Creates new DbAttacher instance for given database.
         * @param db Database to create attacher for.
         * @return Attacher instance.
         */
        DbAttacher* createDbAttacher(Db* db);

        bool isGuiAvailable() const;

        Config* getConfig() const;
        void setConfig(Config* value);

        DbManager* getDbManager() const;
        void setDbManager(DbManager* value);

        FunctionManager* getFunctionManager() const;
        void setFunctionManager(FunctionManager* value);

        PluginManager* getPluginManager() const;
        void setPluginManager(PluginManager* value);

        DbAttacherFactory* getDbAttacherFactory() const;
        void setDbAttacherFactory(DbAttacherFactory* value);

        CollationManager* getCollationManager() const;
        void setCollationManager(CollationManager* value);

        ExportManager* getExportManager() const;
        void setExportManager(ExportManager* value);

        int getVersion() const;
        QString getVersionString() const;

        ImportManager* getImportManager() const;
        void setImportManager(ImportManager* value);

        PopulateManager* getPopulateManager() const;
        void setPopulateManager(PopulateManager* value);

        CodeFormatter* getCodeFormatter() const;
        void setCodeFormatter(CodeFormatter* codeFormatter);

        BugReporter* getBugReporter() const;
        void setBugReporter(BugReporter* value);

        QString getHomePage() const;
        QString getForumPage() const;
        QString getUserManualPage() const;
        QString getSqliteDocsPage() const;

        UpdateManager* getUpdateManager() const;
        void setUpdateManager(UpdateManager* value);

        bool getImmediateQuit() const;
        void setImmediateQuit(bool value);

        ExtraLicenseManager* getExtraLicenseManager() const;
        void setExtraLicenseManager(ExtraLicenseManager* value);

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
         * @brief Code formatter service.
         */
        CodeFormatter* codeFormatter = nullptr;

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

        bool guiAvailable = false;
        bool immediateQuit = false;
        Config* config = nullptr;
        DbManager* dbManager = nullptr;
        FunctionManager* functionManager = nullptr;
        PluginManager* pluginManager = nullptr;
        DbAttacherFactory* dbAttacherFactory = nullptr;
        CollationManager* collationManager = nullptr;
        ExportManager* exportManager = nullptr;
        ImportManager* importManager = nullptr;
        PopulateManager* populateManager = nullptr;
        BugReporter* bugReporter = nullptr;
        UpdateManager* updateManager = nullptr;
        ExtraLicenseManager* extraLicenseManager = nullptr;

    private slots:
        void pluginLoaded(Plugin* plugin,PluginType* pluginType);
        void pluginToBeUnloaded(Plugin* plugin,PluginType* pluginType);
        void pluginUnloaded(const QString& pluginName,PluginType* pluginType);

        /**
         * @brief Cleans up all internal objects.
         *
         * Deletes all internal objects. It's called from destructor.
         */
        void cleanUp();

    public slots:
        /**
         * @brief Updates code formatter with available plugins.
         *
         * Calls CodeFormatter's fullUpdate() method to read available formatters.
         * This also reads formatters selected in config.
         */
        void updateCodeFormatter();

        /**
         * @brief Updates code formater with selected plugins.
         *
         * Doesn't change list of available formatters, but reads new selected formatters from config.
         */
        void updateCurrentCodeFormatter();
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
