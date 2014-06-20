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
        QString getFilePath(Plugin*) const;
        bool loadBuiltInPlugin(Plugin*);
        bool load(const QString&);
        void unload(const QString&);
        void unload(Plugin*);
        bool isLoaded(const QString&) const;
        bool isBuiltIn(const QString&) const;
        Plugin*getLoadedPlugin(const QString&) const;
        QStringList getAllPluginNames(PluginType*) const;
        QStringList getAllPluginNames() const;
        PluginType*getPluginType(const QString&) const;
        QString getAuthor(const QString&) const;
        QString getTitle(const QString&) const;
        QString getPrintableVersion(const QString&) const;
        int getVersion(const QString&) const;
        QString getDescription(const QString&) const;
        PluginType*getPluginType(Plugin*) const;
        QList<Plugin*> getLoadedPlugins(PluginType*) const;
        ScriptingPlugin* getScriptingPlugin(const QString&) const;
        QHash<QString, QVariant> readMetaData(const QJsonObject&);
        QString toPrintableVersion(int) const;

    protected:
        void registerPluginType(PluginType*);
};

#endif // PLUGINMANAGERMOCK_H
