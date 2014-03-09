#include "sqlitestudio.h"
#include "db/dbmanager.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "config.h"
#include "utils.h"
#include "utils_sql.h"
#include "completionhelper.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "notifymanager.h"
#include "sqlformatterplugin.h"
#include "sqlformatter.h"
#include "generalpurposeplugin.h"
#include "sqlfunctionplugin.h"
#include "db/dbplugin.h"
#include "unused.h"
#include "functionmanager.h"
#include "scriptingplugin.h"
#include "scriptingqt.h"
#include <QProcessEnvironment>
#include <QThreadPool>

SQLiteStudio* SQLiteStudio::instance = nullptr;

SQLiteStudio::SQLiteStudio()
{
}

SQLiteStudio::~SQLiteStudio()
{
    cleanUp();
}

void SQLiteStudio::parseCmdLineArgs()
{
    for (int i = 0; i < cmdLineArgs.size(); i++)
    {
        if (cmdLineArgs[i] == "-d")
        {
            // TODO
        }
    }
}

NotifyManager *SQLiteStudio::getNotifyManager() const
{
    return notifyManager;
}

FunctionManager*SQLiteStudio::getFunctionManager() const
{
    return functionManager;
}

SqlFormatter *SQLiteStudio::getSqlFormatter() const
{
    return sqlFormatter;
}

SQLiteStudio *SQLiteStudio::getInstance()
{
    if (!instance)
    {
        instance = new SQLiteStudio();
    }

    return instance;
}

DbManager *SQLiteStudio::getDbManager() const
{
    return dbManager;
}

PluginManager *SQLiteStudio::getPluginManager() const
{
    return pluginManager;
}

Config *SQLiteStudio::getConfig() const
{
    return config;
}

void SQLiteStudio::init(const QStringList& cmdListArguments)
{
    env = new QProcessEnvironment(QProcessEnvironment::systemEnvironment());

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    initUtils();
    CfgMain::staticInit();
    Db::metaInit();
    initUtilsSql();
    initKeywords();
    Lexer::staticInit();
    CompletionHelper::init();

    qRegisterMetaType<ScriptingPlugin::Context*>();

    notifyManager = new NotifyManager();

    config = new Config();
    config->init();

    pluginManager = new PluginManager();
    pluginManager->registerPluginType<GeneralPurposePlugin>(QObject::tr("General purpose"));
    pluginManager->registerPluginType<DbPlugin>(QObject::tr("Database support"));
    pluginManager->registerPluginType<SqlFormatterPlugin>(QObject::tr("SQL formatter"), "formatterPluginsPage");
    pluginManager->registerPluginType<SqlFunctionPlugin>(QObject::tr("SQL function"));
    pluginManager->registerPluginType<ScriptingPlugin>(QObject::tr("Scripting languages"));

    PLUGINS->loadBuiltInPlugin(new ScriptingQt);

    sqlFormatter = new SqlFormatter();
    connect(CFG_CORE.General.ActiveSqlFormatter, SIGNAL(changed(QVariant)), this, SLOT(updateSqlFormatter()));
    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), this, SLOT(updateSqlFormatter()));

    // FunctionManager needs to be set up before DbManager, cause when DbManager starts up, databases make their
    // connections and register functions.
    functionManager = new FunctionManager();

    dbManager = new DbManager();
    cmdLineArgs = cmdListArguments;

    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), dbManager, SLOT(loadDbListFromConfig()));

    pluginManager->init();

    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));

    parseCmdLineArgs();
}

void SQLiteStudio::cleanUp()
{
    if (functionManager)
    {
        delete functionManager;
        functionManager = nullptr;
    }

    if (dbManager)
    {
        delete dbManager;
        dbManager = nullptr;
    }

    if (config)
    {
        delete config;
        config = nullptr;
    }

    if (pluginManager)
    {
        delete pluginManager;
        pluginManager = nullptr;
    }

    if (sqlFormatter)
    {
        delete sqlFormatter;
        sqlFormatter = nullptr;
    }

    if (env)
    {
        delete env;
        env = nullptr;
    }

    if (notifyManager)
    {
        delete notifyManager;
        notifyManager = nullptr;
    }
}

void SQLiteStudio::updateSqlFormatter()
{
    QList<SqlFormatterPlugin *> sqlFormatterPlugins = pluginManager->getLoadedPlugins<SqlFormatterPlugin>();
    QString activeFormatterName = CFG_CORE.General.ActiveSqlFormatter.get();
    foreach (SqlFormatterPlugin* plugin, sqlFormatterPlugins)
    {
        if (plugin->getName() == activeFormatterName)
        {
            sqlFormatter->setFormatter(plugin);
            return;
        }
    }
    sqlFormatter->setFormatter(nullptr);
}

void SQLiteStudio::pluginLoaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    if (pluginType->isForPluginType<SqlFormatterPlugin>())
        updateSqlFormatter();
}

void SQLiteStudio::pluginToBeUnloaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
}

void SQLiteStudio::pluginUnloaded(const QString& pluginName, PluginType* pluginType)
{
    UNUSED(pluginName);
    if (pluginType->isForPluginType<SqlFormatterPlugin>())
        updateSqlFormatter();
}

QString SQLiteStudio::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}
