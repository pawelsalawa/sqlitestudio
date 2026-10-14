#include "letos.h"
#include "config_builder/cfgmain.h"
#include "plugins/plugin.h"
#include "services/codesnippetmanager.h"
#include "services/impl/configimpl.h"
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
#include "services/functionmanager.h"
#include "plugins/scriptingplugin.h"
#include "plugins/exportplugin.h"
#include "plugins/scriptingqt.h"
#include "plugins/dbpluginsqlite3.h"
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
#include "config_builder/cfglazyinitializer.h"
#include "services/config.h"
#include <QProcessEnvironment>
#include <QThreadPool>
#include <QCoreApplication>

DEFINE_SINGLETON(Letos)

static const int letosVersion = 40004;

Letos::Letos()
{
    if (qApp) // qApp is null in unit tests
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

Letos::~Letos()
{
}

void Letos::setupCrashHandler()
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

QStringList Letos::getInitialTranslationFiles() const
{
    return initialTranslationFiles;
}

void Letos::setInitialTranslationFiles(const QStringList& value)
{
    initialTranslationFiles = value;
}

void Letos::installCrashHandler(Letos::CrashHandler handler)
{
    crashHandlers << handler;
}


QString Letos::getCurrentLang() const
{
    return currentLang;
}

ExtraLicenseManager* Letos::getExtraLicenseManager() const
{
    return extraLicenseManager;
}

void Letos::setExtraLicenseManager(ExtraLicenseManager* value)
{
    extraLicenseManager = value;
}


bool Letos::getImmediateQuit() const
{
    return immediateQuit;
}

void Letos::setImmediateQuit(bool value)
{
    immediateQuit = value;
}

#ifdef HAS_UPDATEMANAGER
UpdateManager* Letos::getUpdateManager() const
{
    return updateManager;
}

void Letos::setUpdateManager(UpdateManager* value)
{
    updateManager = value;
}
#endif

PopulateManager* Letos::getPopulateManager() const
{
    return populateManager;
}

void Letos::setPopulateManager(PopulateManager* value)
{
    populateManager = value;
}

CodeFormatter* Letos::getCodeFormatter() const
{
    return codeFormatter;
}

void Letos::setCodeFormatter(CodeFormatter* codeFormatter)
{
    this->codeFormatter = codeFormatter;
}

QString Letos::getHomePage() const
{
    static_qstring(url, "https://letos.org");
    return url;
}

QString Letos::getGitHubReleases() const
{
    static_qstring(url, "https://github.com/pawelsalawa/letos/releases");
    return url;
}

QString Letos::getUserManualPage() const
{
    static_qstring(url, "https://github.com/pawelsalawa/letos/wiki/User_Manual");
    return url;
}

QString Letos::getSqliteDocsPage() const
{
    static_qstring(url, "http://sqlite.org/lang.html");
    return url;
}

QString Letos::getIssuesPage() const
{
    static_qstring(url, "https://github.com/pawelsalawa/letos/issues");
    return url;
}

QString Letos::getDonatePage() const
{
    static_qstring(url, "https://letos.org/donate/");
    return url;
}

QString Letos::getNewIssuePage() const
{
    static_qstring(url, "https://github.com/pawelsalawa/letos/issues/new");
    return url;
}

ImportManager* Letos::getImportManager() const
{
    return importManager;
}

void Letos::setImportManager(ImportManager* value)
{
    importManager = value;
}

ExportManager* Letos::getExportManager() const
{
    return exportManager;
}

void Letos::setExportManager(ExportManager* value)
{
    exportManager = value;
}

CodeSnippetManager* Letos::getCodeSnippetManager() const
{
    return codeSnippetManager;
}

void Letos::setCodeSnippetManager(CodeSnippetManager* newCodeSnippetManager)
{
    codeSnippetManager = newCodeSnippetManager;
}

int Letos::getVersion() const
{
    return letosVersion;
}

QString Letos::getVersionString() const
{
    int ver = getVersion();
    int majorVer = ver / 10000;
    int minorVer = ver % 10000 / 100;
    int patchVer = ver % 100;
    return QString::number(majorVer) + "." + QString::number(minorVer) + "." + QString::number(patchVer);
}

CollationManager* Letos::getCollationManager() const
{
    return collationManager;
}

void Letos::setCollationManager(CollationManager* value)
{
    safe_delete(collationManager);
    collationManager = value;
}

SqliteExtensionManager* Letos::getSqliteExtensionManager() const
{
    return extensionManager;
}

void Letos::setSqliteExtensionManager(SqliteExtensionManager* value)
{
    safe_delete(extensionManager);
    extensionManager = value;
}

DbAttacherFactory* Letos::getDbAttacherFactory() const
{
    return dbAttacherFactory;
}

void Letos::setDbAttacherFactory(DbAttacherFactory* value)
{
    safe_delete(dbAttacherFactory);
    dbAttacherFactory = value;
}

PluginManager* Letos::getPluginManager() const
{
    return pluginManager;
}

void Letos::setPluginManager(PluginManager* value)
{
    safe_delete(pluginManager);
    pluginManager = value;
}

FunctionManager* Letos::getFunctionManager() const
{
    return functionManager;
}

void Letos::setFunctionManager(FunctionManager* value)
{
    safe_delete(functionManager);
    functionManager = value;
}

DbManager* Letos::getDbManager() const
{
    return dbManager;
}

void Letos::setDbManager(DbManager* value)
{
    safe_delete(dbManager);
    dbManager = value;
}

Config* Letos::getConfig() const
{
    return config;
}

void Letos::setConfig(Config* value)
{
    safe_delete(config);
    config = value;
}

void Letos::init(const QStringList& cmdListArguments, bool guiAvailable)
{
    env = new QProcessEnvironment(QProcessEnvironment::systemEnvironment());
    this->guiAvailable = guiAvailable;

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    SQLS_INIT_RESOURCE(core);

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
    qRegisterMetaType<Table>();
    qRegisterMetaType<DbAndTable>();
    qRegisterMetaType<AliasedTable>();
    qRegisterMetaType<Column>();
    qRegisterMetaType<AliasedColumn>();

    NotifyManager::getInstance();

    dbAttacherFactory = new DbAttacherDefaultFactory();

    config = new ConfigImpl();
    config->init();

    currentLang = CFG_CORE.General.Language.get();
    loadTranslations(initialTranslationFiles);
    QLocale locale(currentLang);
    QLocale::setDefault(locale);

    pluginManager = new PluginManagerImpl();

    functionManager = new FunctionManagerImpl();
    collationManager = new CollationManagerImpl();
    extensionManager = new SqliteExtensionManagerImpl();
    dbManager = new DbManagerImpl();

    functionManager->init();
    collationManager->init();
    extensionManager->init();
    dbManager->init();

    pluginManager->registerPluginType<GeneralPurposePlugin>(QObject::tr("General purpose", "plugin category name"));
    pluginManager->registerPluginType<DbPlugin>(QObject::tr("Database support", "plugin category name"));
    pluginManager->registerPluginType<CodeFormatterPlugin>(QObject::tr("Code formatter", "plugin category name"), "formatterPluginsPage");
    pluginManager->registerPluginType<ScriptingPlugin>(QObject::tr("Scripting languages", "plugin category name"),
                                                       "scriptingPluginsPage");
    pluginManager->registerPluginType<ExportPlugin>(QObject::tr("Exporting", "plugin category name"));
    pluginManager->registerPluginType<ImportPlugin>(QObject::tr("Importing", "plugin category name"));
    pluginManager->registerPluginType<PopulatePlugin>(QObject::tr("Table populating", "plugin category name"));

    codeFormatter = new CodeFormatter();
    connect(CFG_CORE.General.ActiveCodeFormatter, SIGNAL(changed(QVariant)), this, SLOT(updateCurrentCodeFormatter()));
    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), this, SLOT(updateCodeFormatter()));

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
#ifdef HAS_UPDATEMANAGER
    updateManager = new UpdateManager();
#endif
    extraLicenseManager = new ExtraLicenseManager();
    codeSnippetManager = new CodeSnippetManager(config);

    extraLicenseManager->addLicense("Letos license (GPL v3)", ":/docs/licenses/letos_license.txt");
    extraLicenseManager->addLicense("Qt, QHexEdit (LGPL v2.1)", ":/docs/licenses/lgpl.txt");
    extraLicenseManager->addLicense("diff_match (Apache License v2.0)", ":/docs/licenses/diff_match.txt");
    extraLicenseManager->addLicense("RSA library (GPL v3)", ":/docs/licenses/gpl.txt");
    extraLicenseManager->addLicense("SingleApplication (The MIT License)", ":/docs/licenses/mit.txt");
    extraLicenseManager->addLicense("ICU (ICU License)", ":/docs/licenses/icu.txt");

    setupCrashHandler();
}

void Letos::initPlugins()
{
    pluginManager->init();

    connect(pluginManager, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(pluginManager, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
    connect(pluginManager, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));
}

void Letos::cleanUp()
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
//#ifdef HAS_UPDATEMANAGER
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
//    SQLS_CLEANUP_RESOURCE(core);
}

void Letos::updateCodeFormatter()
{
    codeFormatter->fullUpdate();
}

void Letos::updateCurrentCodeFormatter()
{
    codeFormatter->updateCurrent();
}

void Letos::pluginLoaded(Plugin* plugin, PluginType* pluginType)
{
    Q_UNUSED(plugin);
    if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
        updateCodeFormatter();
}

void Letos::pluginToBeUnloaded(Plugin* plugin, PluginType* pluginType)
{
    Q_UNUSED(plugin);
    Q_UNUSED(pluginType);
}

void Letos::pluginUnloaded(const QString& pluginName, PluginType* pluginType)
{
    Q_UNUSED(pluginName);
    if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
        updateCodeFormatter();
}

QString Letos::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}

DbAttacher* Letos::createDbAttacher(Db* db)
{
    return dbAttacherFactory->create(db);
}

bool Letos::isGuiAvailable() const
{
    return guiAvailable;
}
