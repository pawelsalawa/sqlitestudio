#ifndef PLUGINMANAGERMOCK_H
#define PLUGINMANAGERMOCK_H

#include "services/pluginmanager.h"

class PluginManagerMock : public PluginManager
{
    public:
        void init();
        void deinit();
        QList<PluginType*> getPluginTypes() const;
        QStringList getPluginDirs() const;
        QString getFilePath(Plugin* plugin) const;
        bool loadBuiltInPlugin(Plugin* plugin);
        bool load(const QString& pluginName);
        void unload(const QString& pluginName);
        void unload(Plugin* plugin);
        bool isLoaded(const QString& pluginName) const;
        bool isBuiltIn(const QString& pluginName) const;
        Plugin*getLoadedPlugin(const QString& pluginName) const;
        QStringList getAllPluginNames(PluginType* type) const;
        QStringList getAllPluginNames() const;
        PluginType*getPluginType(const QString& pluginName) const;
        QString getAuthor(const QString& pluginName) const;
        QString getTitle(const QString& pluginName) const;
        QString getPrintableVersion(const QString& pluginName) const;
        int getVersion(const QString& pluginName) const;
        QString getDescription(const QString& pluginName) const;
        PluginType*getPluginType(Plugin* plugin) const;
        QList<Plugin*> getLoadedPlugins(PluginType* type) const;

    protected:
        void registerPluginType(PluginType* type);
};

#endif // PLUGINMANAGERMOCK_H
