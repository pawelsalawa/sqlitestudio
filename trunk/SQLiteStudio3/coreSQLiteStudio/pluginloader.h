#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "coreSQLiteStudio_global.h"
#include <QPluginLoader>
#include <QList>
#include <QHash>
#include <QDebug>

class Plugin;

class API_EXPORT AbstractPluginLoader
{
    public:
        virtual ~AbstractPluginLoader();

        virtual bool add(QPluginLoader* loader, Plugin* plugin);
        virtual bool test(Plugin* plugin) = 0;
        QList<Plugin*> getPlugins() const;

    private:
        QHash<QPluginLoader*,Plugin*> plugins;
};

template <class T>
class PluginLoader : public AbstractPluginLoader
{
    public:
        bool test(Plugin* plugin)
        {
            return (dynamic_cast<T*>(plugin) != nullptr);
        }
};

#endif // PLUGINLOADER_H
