#include "pluginmanagerimpl.h"
#include "plugins/scriptingplugin.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <plugins/genericplugin.h>

PluginManagerImpl::PluginManagerImpl()
{
}

PluginManagerImpl::~PluginManagerImpl()
{
}

void PluginManagerImpl::init()
{
    pluginDirs += qApp->applicationDirPath() + "/plugins";
    pluginDirs += QDir(CFG->getConfigDir()).absoluteFilePath("plugins");

    QString envDirs = SQLITESTUDIO->getEnv("SQLITESTUDIO_PLUGINS");
    if (!envDirs.isNull())
        pluginDirs += envDirs.split(PATH_LIST_SEPARATOR);

#ifdef PLUGINS_DIR
    pluginDirs += PLUGINS_DIR;
#endif

    scanPlugins();
    loadPlugins();
}

void PluginManagerImpl::deinit()
{
    // Plugin containers and their plugins
    foreach (PluginContainer* container, pluginContainer.values())
    {
        if (container->builtIn)
        {
            container->plugin->deinit();
            delete container->plugin;
        }
        else
            unload(container->name);
    }

    foreach (PluginContainer* container, pluginContainer.values())
        delete container;

    pluginContainer.clear();

    // Types
    foreach (PluginType* type, registeredPluginTypes)
        delete type;

    registeredPluginTypes.clear();
    pluginCategories.clear();
}

QList<PluginType*> PluginManagerImpl::getPluginTypes() const
{
    return registeredPluginTypes;
}

QStringList PluginManagerImpl::getPluginDirs() const
{
    return pluginDirs;
}

QString PluginManagerImpl::getFilePath(Plugin* plugin) const
{
    if (!pluginContainer.contains(plugin->getName()))
        return QString::null;

    return pluginContainer[plugin->getName()]->filePath;
}

bool PluginManagerImpl::loadBuiltInPlugin(Plugin* plugin)
{
    bool res = initPlugin(plugin);
    res &= plugin->init();
    return res;
}

PluginType* PluginManagerImpl::getPluginType(Plugin* plugin) const
{
    if (!pluginContainer.contains(plugin->getName()))
        return nullptr;

    return pluginContainer[plugin->getName()]->type;
}

void PluginManagerImpl::scanPlugins()
{
    QStringList nameFilters;
    nameFilters << "*.so" << "*.dll";

    QPluginLoader* loader;
    foreach (QString pluginDirPath, pluginDirs)
    {
        QDir pluginDir(pluginDirPath);
        foreach (QString fileName, pluginDir.entryList(nameFilters, QDir::Files))
        {
            fileName = pluginDir.absoluteFilePath(fileName);
            loader = new QPluginLoader(fileName);
            loader->setLoadHints(QLibrary::ExportExternalSymbolsHint);

            if (!initPlugin(loader, fileName))
            {
                qDebug() << "File" << fileName << "was loaded as plugin, but SQLiteStudio couldn't initialize plugin.";
                delete loader;
            }
        }
    }

    QStringList names;
    for (PluginContainer* container : pluginContainer.values())
    {
        if (!container->builtIn)
            names << container->name;
    }

    qDebug() << "Following plugins found:" << names;
}

void PluginManagerImpl::loadPlugins()
{
    for (const QString& pluginName : pluginContainer.keys())
    {
        if (shouldAutoLoad(pluginName))
            load(pluginName);
    }

    emit pluginsInitiallyLoaded();
}

bool PluginManagerImpl::initPlugin(QPluginLoader* loader, const QString& fileName)
{
    QJsonObject pluginMetaData = loader->metaData();
    QString pluginTypeName = pluginMetaData.value("MetaData").toObject().value("type").toString();
    PluginType* pluginType = nullptr;
    foreach (PluginType* type, registeredPluginTypes)
    {
        if (type->getName() == pluginTypeName)
        {
            pluginType = type;
            break;
        }
    }

    if (!pluginType)
    {
        qWarning() << "Could not load plugin" + fileName + "because its type was not recognized:" << pluginTypeName;
        return false;
    }

    QString pluginName = pluginMetaData.value("className").toString();

    PluginContainer* container = new PluginContainer;
    container->type = pluginType;
    container->filePath = fileName;
    container->loaded = false;
    container->loader = loader;
    pluginCategories[pluginType] << container;
    pluginContainer[pluginName] = container;
    for (const QJsonValue& value : pluginMetaData.value("MetaData").toObject().value("dependencies").toArray())
        container->dependencies << value.toString();

    if (!readMetaData(container))
    {
        delete container;
        return false;
    }

    return true;
}

bool PluginManagerImpl::initPlugin(Plugin* plugin)
{
    QString pluginName = plugin->getName();
    PluginType* pluginType = nullptr;
    foreach (PluginType* type, registeredPluginTypes)
    {
        if (type->test(plugin))
        {
            pluginType = type;
            break;
        }
    }

    if (!pluginType)
    {
        qWarning() << "Could not load built-in plugin" + pluginName + "because its type was not recognized.";
        return false;
    }

    PluginContainer* container = new PluginContainer;
    container->type = pluginType;
    container->loaded = true;
    container->builtIn = true;
    container->plugin = plugin;
    pluginCategories[pluginType] << container;
    pluginContainer[pluginName] = container;
    if (!readMetaData(container))
    {
        delete container;
        return false;
    }

    pluginLoaded(container);
    return true;
}

bool PluginManagerImpl::shouldAutoLoad(const QString& pluginName)
{
    QStringList loadedPlugins = CFG_CORE.General.LoadedPlugins.get().split(",", QString::SkipEmptyParts);
    QStringList pair;
    foreach (const QString& loadedPlugin, loadedPlugins)
    {
        pair = loadedPlugin.split("=");
        if (pair.size() != 2)
        {
            qWarning() << "Invalid entry in config General.LoadedPlugins:" << loadedPlugin;
            continue;
        }

        if (pair[0] == pluginName)
            return (bool)pair[1].toInt();
    }

    return true;
}

QStringList PluginManagerImpl::getAllPluginNames(PluginType* type) const
{
    QStringList names;
    if (!pluginCategories.contains(type))
        return names;

    foreach (PluginContainer* container, pluginCategories[type])
        names << container->name;

    return names;
}

QStringList PluginManagerImpl::getAllPluginNames() const
{
    return pluginContainer.keys();
}

PluginType* PluginManagerImpl::getPluginType(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return nullptr;

    return pluginContainer[pluginName]->type;
}

QString PluginManagerImpl::getAuthor(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->author;
}

QString PluginManagerImpl::getTitle(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->title;
}

QString PluginManagerImpl::getPrintableVersion(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->printableVersion;
}

int PluginManagerImpl::getVersion(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return 0;

    return pluginContainer[pluginName]->version;
}

QString PluginManagerImpl::getDescription(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->description;
}

void PluginManagerImpl::unload(Plugin* plugin)
{
    if (!plugin)
        return;

    unload(plugin->getName());
}

void PluginManagerImpl::unload(const QString& pluginName)
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to unload plugin.";
        return;
    }

    // Checking preconditions
    PluginContainer* container = pluginContainer[pluginName];
    if (container->builtIn)
        return;

    if (!container->loaded)
        return;

    // Unloading depdendent plugins
    for (PluginContainer* otherContainer : pluginContainer.values())
    {
        if (otherContainer == container)
            continue;

        if (otherContainer->dependencies.contains(pluginName))
            unload(otherContainer->name);
    }

    // Removing from fast-lookup collections
    removePluginFromCollections(container->plugin);

    // Deinitializing and unloading plugin
    emit aboutToUnload(container->plugin, container->type);
    container->plugin->deinit();

    QPluginLoader* loader = container->loader;
    if (!loader->isLoaded())
    {
        qWarning() << "QPluginLoader says the plugin is not loaded. Weird.";
        emit unloaded(container->name, container->type);
        return;
    }

    loader->unload();

    container->plugin = nullptr;
    container->loaded = false;

    emit unloaded(container->name, container->type);

    qDebug() << pluginName << "unloaded:" << container->filePath;
}

bool PluginManagerImpl::load(const QString& pluginName)
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to load plugin.";
        return false;
    }

    PluginContainer* container = pluginContainer[pluginName];
    if (container->builtIn)
        return true;

    QPluginLoader* loader = container->loader;
    if (loader->isLoaded())
        return true;

    for (const QString& dep : container->dependencies)
    {
        if (!load(dep))
        {
            qWarning() << "Could not load dependency" << dep << "for plugin" << pluginName << ", so it won't be loaded.";
            return false;
        }
    }

    if (!loader->load())
    {
        qWarning() << "Could not load plugin file:" << loader->errorString();
        return false;
    }

    Plugin* plugin = dynamic_cast<Plugin*>(container->loader->instance());
    GenericPlugin* genericPlugin = dynamic_cast<GenericPlugin*>(plugin);
    if (genericPlugin)
    {
        genericPlugin->loadMetaData(container->loader->metaData());
    }

    if (!plugin->init())
    {
        qWarning() << "Error initializing plugin:" << container->name;
        return false;
    }

    pluginLoaded(container);

    return true;
}

void PluginManagerImpl::pluginLoaded(PluginManagerImpl::PluginContainer* container)
{
    if (!container->builtIn)
    {
        container->plugin = dynamic_cast<Plugin*>(container->loader->instance());
        container->loaded = true;
    }
    addPluginToCollections(container->plugin);

    emit loaded(container->plugin, container->type);
    if (!container->builtIn)
        qDebug() << container->name << "loaded:" << container->filePath;
}

void PluginManagerImpl::addPluginToCollections(Plugin* plugin)
{
    ScriptingPlugin* scriptingPlugin = dynamic_cast<ScriptingPlugin*>(plugin);
    if (scriptingPlugin)
        scriptingPlugins[scriptingPlugin->getLanguage()] = scriptingPlugin;
}

void PluginManagerImpl::removePluginFromCollections(Plugin* plugin)
{
    ScriptingPlugin* scriptingPlugin = dynamic_cast<ScriptingPlugin*>(plugin);
    if (scriptingPlugin && scriptingPlugins.contains(scriptingPlugin->getLanguage()))
        scriptingPlugins.remove(plugin->getName());
}

bool PluginManagerImpl::readMetaData(PluginManagerImpl::PluginContainer* container)
{
    if (container->loader)
    {
        QHash<QString, QVariant> metaData = readMetaData(container->loader->metaData());
        container->name = metaData["name"].toString();
        container->version = metaData["version"].toInt();
        container->printableVersion = toPrintableVersion(metaData["version"].toInt());
        container->author = metaData["author"].toString();
        container->description = metaData["description"].toString();
        container->title = metaData["title"].toString();
    }
    else if (container->plugin)
    {
        container->name = container->plugin->getName();
        container->version = container->plugin->getVersion();
        container->printableVersion = container->plugin->getPrintableVersion();
        container->author = container->plugin->getAuthor();
        container->description = container->plugin->getDescription();
        container->title = container->plugin->getTitle();
    }
    else
    {
        qCritical() << "Could not read metadata for some plugin. It has no loader or plugin object defined.";
        return false;
    }
    return true;
}

bool PluginManagerImpl::isLoaded(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to get plugin 'loaded' status.";
        return false;
    }

    return pluginContainer[pluginName]->loaded;
}

bool PluginManagerImpl::isBuiltIn(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to get plugin 'builtIn' status.";
        return false;
    }

    return pluginContainer[pluginName]->builtIn;
}

Plugin* PluginManagerImpl::getLoadedPlugin(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return nullptr;

    if (!pluginContainer[pluginName]->loaded)
        return nullptr;

    return pluginContainer[pluginName]->plugin;
}

QList<Plugin*> PluginManagerImpl::getLoadedPlugins(PluginType* type) const
{
    QList<Plugin*> list;
    if (!pluginCategories.contains(type))
        return list;

    foreach (PluginContainer* container, pluginCategories[type])
    {
        if (container->loaded)
            list << container->plugin;
    }

    return list;
}

ScriptingPlugin* PluginManagerImpl::getScriptingPlugin(const QString& languageName) const
{
    if (scriptingPlugins.contains(languageName))
        return scriptingPlugins[languageName];

    return nullptr;
}

QHash<QString, QVariant> PluginManagerImpl::readMetaData(const QJsonObject& metaData)
{
    QHash<QString, QVariant> results;
    results["name"] = metaData.value("className").toString();

    QJsonObject root = metaData.value("MetaData").toObject();
    results["type"] = root.value("type").toString();
    results["title"] = root.value("title").toString();
    results["description"] = root.value("description").toString();
    results["author"] = root.value("author").toString();
    results["version"] = root.value("version").toInt();
    results["ui"] = root.value("ui").toString();
    return results;
}

QString PluginManagerImpl::toPrintableVersion(int version) const
{
    static const QString versionStr = QStringLiteral("%1.%2.%3");
    return versionStr.arg(version / 10000)
                     .arg(version / 100 % 100)
                     .arg(version % 100);
}

void PluginManagerImpl::registerPluginType(PluginType* type)
{
    registeredPluginTypes << type;
}
