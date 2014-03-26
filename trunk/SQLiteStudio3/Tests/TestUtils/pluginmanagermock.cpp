#include "pluginmanagermock.h"

void PluginManagerMock::init()
{
}

void PluginManagerMock::deinit()
{
}

QList<PluginType*> PluginManagerMock::getPluginTypes() const
{
    return QList<PluginType*>();
}

QStringList PluginManagerMock::getPluginDirs() const
{
    return QStringList();
}

QString PluginManagerMock::getFilePath(Plugin* plugin) const
{
    return QString();
}

bool PluginManagerMock::loadBuiltInPlugin(Plugin* plugin)
{
    return true;
}

bool PluginManagerMock::load(const QString& pluginName)
{
    return true;
}

void PluginManagerMock::unload(const QString& pluginName)
{
}

void PluginManagerMock::unload(Plugin* plugin)
{
}

bool PluginManagerMock::isLoaded(const QString& pluginName) const
{
    return false;
}

bool PluginManagerMock::isBuiltIn(const QString& pluginName) const
{
    return false;
}

Plugin* PluginManagerMock::getLoadedPlugin(const QString& pluginName) const
{
    return nullptr;
}

QStringList PluginManagerMock::getAllPluginNames(PluginType* type) const
{
    return QStringList();
}

QStringList PluginManagerMock::getAllPluginNames() const
{
    return QStringList();
}

PluginType* PluginManagerMock::getPluginType(const QString& pluginName) const
{
    return nullptr;
}

QString PluginManagerMock::getAuthor(const QString& pluginName) const
{
    return QString();
}

QString PluginManagerMock::getTitle(const QString& pluginName) const
{
    return QString();
}

QString PluginManagerMock::getPrintableVersion(const QString& pluginName) const
{
    return QString();
}

int PluginManagerMock::getVersion(const QString& pluginName) const
{
    return 3;
}

QString PluginManagerMock::getDescription(const QString& pluginName) const
{
    return QString();
}

PluginType* PluginManagerMock::getPluginType(Plugin* plugin) const
{
    return nullptr;
}

QList<Plugin*> PluginManagerMock::getLoadedPlugins(PluginType* type) const
{
    return QList<Plugin*>();
}

ScriptingPlugin* PluginManagerMock::getScriptingPlugin(const QString& languageName) const
{
    return nullptr;
}

void PluginManagerMock::registerPluginType(PluginType* type)
{
}
