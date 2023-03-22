#include "sqlitestudio.h"
#include "plugins/plugin.h"
#include "services/codesnippetmanager.h"
#include "services/pluginmanager.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "completionhelper.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "services/notifymanager.h"
#include "plugins/codeformatterplugin.h"
#include "services/codeformatter.h"
#include "plugins/generalpurposeplugin.h"
#include "plugins/dbplugin.h"
#include "common/unused.h"
#include "services/functionmanager.h"
#include "plugins/scriptingplugin.h"
#include "plugins/exportplugin.h"
#include "plugins/scriptingqt.h"
#include "plugins/dbpluginsqlite3.h"
#include "services/impl/configimpl.h"
#include "services/impl/dbmanagerimpl.h"
#include "services/impl/functionmanagerimpl.h"
#include "services/impl/collationmanagerimpl.h"
#include "services/impl/pluginmanagerimpl.h"
#include "services/impl/sqliteextensionmanagerimpl.h"
#include "services/updatemanager.h"
#include "impl/dbattacherimpl.h"
#include "services/exportmanager.h"
#include "services/importmanager.h"
#include "services/populatemanager.h"
#include "plugins/scriptingsql.h"
#include "plugins/importplugin.h"
#include "plugins/populateplugin.h"
#include "services/extralicensemanager.h"
#include "services/sqliteextensionmanager.h"
#include "translations.h"
#include "chillout/chillout.h"
#include <QProcessEnvironment>
#include <QThreadPool>
#include <QCoreApplication>

DEFINE_SINGLETON(SQLiteStudio)

static const int sqlitestudioVersion = 30404;

SQLiteStudio::SQLiteStudio()
{
    if (qApp) // qApp is null in unit tests
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

SQLiteStudio::~SQLiteStudio()
{
}

void SQLiteStudio::setupCrashHandler()
{
    auto &chillout = Debug::Chillout::getInstance();

#ifdef _WIN32
    chillout.init(qApp->applicationName().toStdWString(), qApp->applicationDirPath().toStdWString());
#else
    chillout.init(qApp->applicationName().toStdString(), qApp->applicationDirPath().toStdString());
#endif

    chillout.setBacktraceCallback([](const char * const) {});

    chillout.setCrashCallback([this]() {
        for (CrashHandler& hnd : crashHandlers)
            hnd();
    });
}

QStringList SQLiteStudio::getInitialTranslationFiles() const
{
    return initialTranslationFiles;
}

void SQLiteStudio::setInitialTranslationFiles(const QStringList& value)
{
    initialTranslationFiles = value;
}

void SQLiteStudio::installCrashHandler(SQLiteStudio::CrashHandler handler)
{
    crashHandlers << handler;
}


QString SQLiteStudio::getCurrentLang() const
{
    return currentLang;
}

ExtraLicenseManager* SQLiteStudio::getExtraLicenseManager() const
{
    return extraLicenseManager;
}

void SQLiteStudio::setExtraLicenseManager(ExtraLicenseManager* value)
{
    extraLicenseManager = value;
}


bool SQLiteStudio::getImmediateQuit() const
{
    return immediateQuit;
}

void SQLiteStudio::setImmediateQuit(bool value)
{
    immediateQuit = value;
}

#ifdef PORTABLE_CONFIG
UpdateManager* SQLiteStudio::getUpdateManager() const
{
    return updateManager;
}

void SQLiteStudio::setUpdateManager(UpdateManager* value)
{
    updateManager = value;
}
#endif

PopulateManager* SQLiteStudio::getPopulateManager() const
{
    return populateManager;
}

void SQLiteStudio::setPopulateManager(PopulateManager* value)
{
    populateManager = value;
}

CodeFormatter* SQLiteStudio::getCodeFormatter() const
{
    return codeFormatter;
}

void SQLiteStudio::setCodeFormatter(CodeFormatter* codeFormatter)
{
    this->codeFormatter = codeFormatter;
}

QString SQLiteStudio::getHomePage() const
{
    static_qstring(url, "https://sqlitestudio.pl");
    return url;
}

QString SQLiteStudio::getGitHubReleases() const
{
    static_qstring(url, "https://github.com/pawelsalawa/sqlitestudio/releases");
    return url;
}

QString SQLiteStudio::getUserManualPage() const
{
    static_qstring(url, "https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual");
    return url;
}

QString SQLiteStudio::getSqliteDocsPage() const
{
    static_qstring(url, "http://sqlite.org/lang.html");
    return url;
}

QString SQLiteStudio::getIssuesPage() const
{
    static_qstring(url, "https://github.com/pawelsalawa/sqlitestudio/issues");
    return url;
}

QString SQLiteStudio::getDonatePage() const
{
    static_qstring(url, "https://sqlitestudio.pl/donate/");
    return url;
}

QString SQLiteStudio::getNewIssuePage() const
{
    static_qstring(url, "https://github.com/pawelsalawa/sqlitestudio/issues/new");
    return url;
}

ImportManager* SQLiteStudio::getImportManager() const
{
    return importManager;
}

void SQLiteStudio::setImportManager(ImportManager* value)
{
    importManager = value;
}

ExportManager* SQLiteStudio::getExportManager() const
{
    return exportManager;
}

void SQLiteStudio::setExportManager(ExportManager* value)
{
    exportManager = value;
}

CodeSnippetManager* SQLiteStudio::getCodeSnippetManager() const
{
    return codeSnippetManager;
}

void SQLiteStudio::setCodeSnippetManager(CodeSnippetManager* newCodeSnippetManager)
{
    codeSnippetManager = newCodeSnippetManager;
}

int SQLiteStudio::getVersion() const
{
    return sqlitestudioVersion;
}

QString SQLiteStudio::getVersionString() const
{
    int ver = getVersion();
    int majorVer = ver / 10000;
    int minorVer = ver % 10000 / 100;
    int patchVer = ver % 100;
    return QString::number(majorVer) + "." + QString::number(minorVer) + "." + QString::number(patchVer);
}

CollationManager* SQLiteStudio::getCollationManager() const
{
    return collationManager;
}

void SQLiteStudio::setCollationManager(CollationManager* value)
{
    safe_delete(collationManager);
    collationManager = value;
}

SqliteExtensionManager* SQLiteStudio::getSqliteExtensionManager() const
{
    return extensionManager;
}

void SQLiteStudio::setSqliteExtensionManager(SqliteExtensionManager* value)
{
    safe_delete(extensionManager);
    extensionManager = value;
}

DbAttacherFactory* SQLiteStudio::getDbAttacherFactory() const
{
    return dbAttacherFactory;
}

void SQLiteStudio::setDbAttacherFactory(DbAttacherFactory* value)
{
    safe_delete(dbAttacherFactory);
    dbAttacherFactory = value;
}

PluginManager* SQLiteStudio::getPluginManager() const
{
    return pluginManager;
}

void SQLiteStudio::setPluginManager(PluginManager* value)
{
    safe_delete(pluginManager);
    pluginManager = value;
}

FunctionManager* SQLiteStudio::getFunctionManager() const
{
    return functionManager;
}

void SQLiteStudio::setFunctionManager(FunctionManager* value)
{
    safe_delete(functionManager);
    functionManager = value;
}

DbManager* SQLiteStudio::getDbManager() const
{
    return dbManager;
}

void SQLiteStudio::setDbManager(DbManager* value)
{
    safe_delete(dbManager);
    dbManager = value;
}

Config* SQLiteStudio::getConfig() const
{
    return config;
}

void SQLiteStudio::setConfig(Config* value)
{
    safe_delete(config);
    config = value;
}

void SQLiteStudio::init(const QStringList& cmdListArguments, bool guiAvailable)
{
    env = new QProcessEnvironment(QProcessEnvironment::systemEnvironment());
    this->guiAvailable = guiAvailable;

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    SQLS_INIT_RESOURCE(coreSQLiteStudio);

    CfgLazyInitializer::init();

    initUtils();
    CfgMain::staticInit();
    Db::metaInit();
    initUtilsSql();
    SchemaResolver::staticInit();
    initKeywords();
    Lexer::staticInit();
    CompletionHelper::init();

    qRegisterMetaType<ScriptingPlugin::Context*>();

    NotifyManager::getInstance();

    dbAttacherFactory = new DbAttacherDefaultFactory();

    config = new ConfigImpl();
    config->init();

    currentLang = CFG_CORE.General.Language.get();
    loadTranslations(initialTranslationFiles);

    pluginManager = new PluginManagerImpl();
    dbManager = new DbManagerImpl();

    pluginManager->registerPluginType<GeneralPurposePlugin>(QObject::tr("General purpose", "plugin category name"));
    pluginManager->registerPluginType<DbPlugin>(QObject::tr("Database support", "plugin category name"));
    pluginManager->registerPluginType<CodeFormatterPlugin>(QObject::tr("Code formatter", "plugin category name"), "formatterPluginsPage");
    pluginManager->registerPluginType<ScriptingPlugin>(QObject::tr("Scripting languages", "plugin category name"));
    pluginManager->registerPluginType<ExportPlugin>(QObject::tr("Exporting", "plugin category name"));
    pluginManager->registerPluginType<ImportPlugin>(QObject::tr("Importing", "plugin category name"));
    pluginManager->registerPluginType<PopulatePlugin>(QObject::tr("Table populating", "plugin category name"));

    codeFormatter = new CodeFormatter();
    connect(CFG_CORE.General.ActiveCodeFormatter, SIGNAL(changed(QVariant)), this, SLOT(updateCurrentCodeFormatter()));
    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), this, SLOT(updateCodeFormatter()));

    // FunctionManager needs to be set up before DbManager, cause when DbManager starts up, databases make their
    // connections and register functions.
    functionManager = new FunctionManagerImpl();

    collationManager = new CollationManagerImpl();
    extensionManager = new SqliteExtensionManagerImpl();

    cmdLineArgs = cmdListArguments;

    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), DBLIST, SLOT(notifyDatabasesAreLoaded()));

    DbPluginSqlite3* sqlite3plugin = new DbPluginSqlite3;
    dynamic_cast<DbManagerImpl*>(dbManager)->setInMemDbCreatorPlugin(sqlite3plugin);

    pluginManager->loadBuiltInPlugin(new ScriptingQt);
    pluginManager->loadBuiltInPlugin(new ScriptingSql);
    pluginManager->loadBuiltInPlugin(sqlite3plugin);

    exportManager = new ExportManager();
    importManager = new ImportManager();
    populateManager = new PopulateManager();
#ifdef PORTABLE_CONFIG
    updateManager = new UpdateManager();
#endif
    extraLicenseManager = new ExtraLicenseManager();
    codeSnippetManager = new CodeSnippetManager(config);

    extraLicenseManager->addLicense("SQLiteStudio license (GPL v3)", ":/docs/licenses/sqlitestudio_license.txt");
    extraLicenseManager->addLicense("Fugue icons", ":/docs/licenses/fugue_icons.txt");
    extraLicenseManager->addLicense("Qt, QHexEdit (LGPL v2.1)", ":/docs/licenses/lgpl.txt");
    extraLicenseManager->addLicense("diff_match (Apache License v2.0)", ":/docs/licenses/diff_match.txt");
    extraLicenseManager->addLicense("RSA library (GPL v3)", ":/docs/licenses/gpl.txt");
    extraLicenseManager->addLicense("SingleApplication (The MIT License)", ":/docs/licenses/mit.txt");
    extraLicenseManager->addLicense("ICU (ICU License)", ":/docs/licenses/icu.txt");

    setupCrashHandler();
}

void SQLiteStudio::initPlugins()
{
    pluginManager->init();

    connect(pluginManager, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(pluginManager, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
    connect(pluginManager, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));
}

void SQLiteStudio::cleanUp()
{
    if (finalCleanupDone)
        return;

    finalCleanupDone = true;
    emit aboutToQuit();
    // Deleting all singletons contained in this object, alongside with plugin deinitialization & unloading
    // causes QTranslator to crash randomly during shutdown, due to some issue in Qt itself, because it tries to refresh
    // some internal translators state after the translator is uninstalled, but at the same time many message resources
    // are being unloaded together with plugins and it somehow causes the crash (randomly).
    // At the same time if hardly find any reason to execute proper deinitialization of all singletons, when the application stops.
    // The session (UI) is saved anyway independently in the UI code.
    // Explicit deletion of singletons does not really have any benefits.
    // Leaving this code here for some time, just to understand it later if needed, but eventually it will be deleted.
//    disconnect(pluginManager, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
//    disconnect(pluginManager, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));
//    if (!immediateQuit)
//    {
//        if (pluginManager)
//            pluginManager->deinit();

//        safe_delete(pluginManager); // PluginManager before DbManager, so Db objects are deleted while DbManager still exists
//#ifdef PORTABLE_CONFIG
//        safe_delete(updateManager);
//#endif
//        safe_delete(populateManager);
//        safe_delete(importManager);
//        safe_delete(exportManager);
//        safe_delete(functionManager);
//        safe_delete(extraLicenseManager);
//        safe_delete(dbManager);
//        safe_delete(config);
//        safe_delete(codeFormatter);
//        safe_delete(dbAttacherFactory);
//        safe_delete(env);
//        NotifyManager::destroy();
//    }
//    SQLS_CLEANUP_RESOURCE(coreSQLiteStudio);
}

void SQLiteStudio::updateCodeFormatter()
{
    codeFormatter->fullUpdate();
}

void SQLiteStudio::updateCurrentCodeFormatter()
{
    codeFormatter->updateCurrent();
}

void SQLiteStudio::pluginLoaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
        updateCodeFormatter();
}

void SQLiteStudio::pluginToBeUnloaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
}

void SQLiteStudio::pluginUnloaded(const QString& pluginName, PluginType* pluginType)
{
    UNUSED(pluginName);
    if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
        updateCodeFormatter();
}

QString SQLiteStudio::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}

DbAttacher* SQLiteStudio::createDbAttacher(Db* db)
{
    return dbAttacherFactory->create(db);
}

bool SQLiteStudio::isGuiAvailable() const
{
    return guiAvailable;
}
