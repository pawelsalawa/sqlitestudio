#include "sqlitestudio.h"
#include "plugins/plugin.h"
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
#include "impl/dbattacherimpl.h"
#include "services/exportmanager.h"
#include "services/importmanager.h"
#include "services/populatemanager.h"
#include "plugins/scriptingsql.h"
#include "plugins/importplugin.h"
#include "plugins/populateplugin.h"
#include <QProcessEnvironment>
#include <QThreadPool>
#include <QCoreApplication>

DEFINE_SINGLETON(SQLiteStudio)

static const int sqlitestudioVersion = 29901;

SQLiteStudio::SQLiteStudio()
{
    if (qApp) // qApp is null in unit tests
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

SQLiteStudio::~SQLiteStudio()
{
}

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
    static const QString url = QStringLiteral("http://sqlitestudio.pl");
    return url;
}

QString SQLiteStudio::getForumPage() const
{
    static const QString url = QStringLiteral("http://forum.sqlitestudio.pl");
    return url;
}

QString SQLiteStudio::getUserManualPage() const
{
    static const QString url = QStringLiteral("http://wiki.sqlitestudio.pl/index.php/User_Manual");
    return url;
}

QString SQLiteStudio::getSqliteDocsPage() const
{
    static const QString url = QStringLiteral("http://sqlite.org/lang.html");
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

    Q_INIT_RESOURCE(coresqlitestudio);

    CfgLazyInitializer::init();

    initUtils();
    CfgMain::staticInit();
    Db::metaInit();
    initUtilsSql();
    initKeywords();
    Lexer::staticInit();
    CompletionHelper::init();

    qRegisterMetaType<ScriptingPlugin::Context*>();

    NotifyManager::getInstance();

    dbAttacherFactory = new DbAttacherDefaultFactory();

    config = new ConfigImpl();
    config->init();

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

    cmdLineArgs = cmdListArguments;

    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), DBLIST, SLOT(loadDbListFromConfig()));

    DbPluginSqlite3* sqlite3plugin = new DbPluginSqlite3;
    dynamic_cast<DbManagerImpl*>(dbManager)->setInMemDbCreatorPlugin(sqlite3plugin);

    pluginManager->loadBuiltInPlugin(new ScriptingQt);
    pluginManager->loadBuiltInPlugin(new ScriptingSql);
    pluginManager->loadBuiltInPlugin(sqlite3plugin);

    exportManager = new ExportManager();
    importManager = new ImportManager();
    populateManager = new PopulateManager();
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
    safe_delete(populateManager);
    safe_delete(importManager);
    safe_delete(exportManager);
    safe_delete(functionManager);
    pluginManager->deinit();
    safe_delete(pluginManager); // PluginManager before DbManager, so Db objects are deleted while DbManager still exists
    safe_delete(dbManager);
    safe_delete(config);
    safe_delete(codeFormatter);
    safe_delete(dbAttacherFactory);
    safe_delete(env);
    NotifyManager::destroy();
    Q_CLEANUP_RESOURCE(coresqlitestudio);
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
