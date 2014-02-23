#include "plugin.h"
#include "pluginloader.h"
#include "unused.h"

AbstractPluginLoader::~AbstractPluginLoader()
{
    foreach (Plugin* plugin, plugins.values())
    {
        plugin->deinit();
    }

    foreach (QPluginLoader* loader, plugins.keys())
    {
        if (!loader->unload())
            qDebug() << "Could not unload plugin" << loader->fileName() << ":" << loader->errorString();

        delete loader;
    }
    plugins.clear();
}

bool AbstractPluginLoader::add(QPluginLoader* loader, Plugin* plugin)
{
    if (!plugin->init())
        return false;

    plugins[loader] = plugin;
    return true;
}


QList<Plugin*> AbstractPluginLoader::getPlugins() const
{
    return plugins.values();
}
