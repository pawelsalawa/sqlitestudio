#ifndef PLUGINMANAGERIMPL_H
#define PLUGINMANAGERIMPL_H

#include "services/pluginmanager.h"
#include <QPluginLoader>
#include <QHash>

class PluginManagerImpl : public PluginManager
{
    Q_OBJECT

    public:
        /**
         * @brief Creates plugin manager.
         */
        PluginManagerImpl();

        /**
         * @brief Deletes plugin manager.
         */
        ~PluginManagerImpl();

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
        Plugin* getLoadedPlugin(const QString& pluginName) const;
        QStringList getAllPluginNames(PluginType* type) const;
        QStringList getAllPluginNames() const;
        PluginType* getPluginType(const QString& pluginName) const;
        QString getAuthor(const QString& pluginName) const;
        QString getTitle(const QString& pluginName) const;
        QString getPrintableVersion(const QString& pluginName) const;
        int getVersion(const QString& pluginName) const;
        QString getDescription(const QString& pluginName) const;
        PluginType* getPluginType(Plugin* plugin) const;
        QList<Plugin*> getLoadedPlugins(PluginType* type) const;
        ScriptingPlugin* getScriptingPlugin(const QString& languageName) const;

    protected:
        void registerPluginType(PluginType* type);

    private:
        /**
         * @brief Container for plugin related data.
         *
         * The container is used to represent plugin available to the application,
         * no matter if it's loaded or not. It keeps all plugin related data,
         * so it's available even the plugin is not loaded.
         */
        struct PluginContainer
        {
            /**
             * @brief Name of the plugin.
             */
            QString name;

            /**
             * @brief Title of the plugin, used on UI.
             */
            QString title;

            /**
             * @brief Plugin's detailed description.
             */
            QString description;

            /**
             * @brief Plugin's author.
             */
            QString author;

            /**
             * @brief Numeric verion of the plugin.
             */
            int version;

            /**
             * @brief Human-readable version.
             */
            QString printableVersion;

            /**
             * @brief Type of the plugin.
             */
            PluginType* type;

            /**
             * @brief Full path to the plugin's file.
             */
            QString filePath;

            /**
             * @brief Plugin's loaded state flag.
             */
            bool loaded;

            /**
             * @brief Qt's plugin framework loaded for this plugin.
             */
            QPluginLoader* loader;

            /**
             * @brief Plugin object.
             *
             * It's null when plugin is not loaded.
             */
            Plugin* plugin;

            /**
             * @brief Flag indicating that the plugin is built in.
             *
             * Plugins built-in are classes implementing plugin's interface,
             * but they are compiled and statically linked to the main application binary.
             * They cannot be loaded or unloaded - they are loaded by default.
             */
            bool builtIn = false;
        };

        /**
         * @brief List of plugins, both loaded and unloaded.
         */
        typedef QList<PluginContainer*> PluginContainerList;

        /**
         * @brief Loads plugins defined in configuration.
         *
         * It loads all plugins that are available to the application
         * and are not marked to not load in the configuration.
         *
         * In other words, every plugin will load by default, unless it was
         * explicitly unloaded previously and that was saved in the configuration
         * (when application was closing).
         */
        void loadPlugins();

        /**
         * @brief Executes standard routines after plugin was loaded.
         * @param container Container for the loaded plugin.
         *
         * It fills all members of the plugin container and emits loaded() signal.
         */
        void pluginLoaded(PluginContainer* container);

        /**
         * @brief Reads title, description, author, etc. from the plugin.
         * @param plugin Plugin to read data from.
         * @param container Container to put the data to.
         *
         * It does the reading by calling all related methods from Plugin interface,
         * then stores those information in given \p container.
         */
        void readMetadata(Plugin* plugin, PluginContainer* container);

        /**
         * @brief Tries to load given file as a Plugin.
         * @param fileName File to load (absolute path).
         * @param loader Qt's plugin framework loader to use when loading the plugin.
         * @return Loaded plugin object, or null if loading failed.
         *
         * It loads plugin file, resolves symbols, creates object delivered by the plugin
         * and casts it to Plugin interface. If it fails on any step, the file is unloaded
         * and the method returns null.
         *
         * The loader should be exclusive for given plugin file, as it will be later
         * associated with the plugin (so the loader can unload the plugin).
         */
        Plugin* loadPluginFromFile(const QString& fileName, QPluginLoader* loader);

        /**
         * @brief Creates plugin container and initializes it.
         * @param loader Qt's plugin framework loader used to load this plugin.
         *               For built-in plugins (statically linked) this must be null.
         * @param fileName Plugin's file path. For built-in plugins it's ignored.
         * @param plugin Plugin object from loaded plugin.
         * @return true if the initialization succeeded, or false otherwise.
         *
         * It assigns plugin type to the plugin, creates plugin container and fills
         * all necessary data for the plugin. If the plugin was configured to not load,
         * then this method unloads the file, before plugin was initialized (with Plugin::init()).
         *
         * All plugins are loaded at the start, but before they are fully initialized
         * and enabled, they are simply queried for metadata, then either unloaded
         * (when configured to not load at startup), or the initialization proceeds.
         */
        bool initPlugin(QPluginLoader* loader, const QString& fileName, Plugin* plugin);

        /**
         * @brief Tests if given plugin is configured to be loaded at startup.
         * @param plugin Tested plugin object.
         * @return true if plugin should be loaded at startup, or false otherwise.
         *
         * This method checks General.LoadedPlugins configuration entry to see if plugin
         * was explicitly disabled for loading at startup.
         */
        bool shouldAutoLoad(Plugin* plugin);

        /**
         * @brief List of plugin directories (not necessarily absolute paths).
         */
        QStringList pluginDirs;

        /**
         * @brief List of registered plugin types.
         */
        QList<PluginType*> registeredPluginTypes;

        /**
         * @brief Table with plugin types as keys and list of plugins assigned for each type.
         */
        QHash<PluginType*,PluginContainerList> pluginCategories;

        /**
         * @brief Table with plugin names and containers assigned for each plugin.
         */
        QHash<QString,PluginContainer*> pluginContainer;

        /**
         * @brief Internal list of scripting plugins, updated on load/unload of plugins.
         *
         * Keys are scripting language name.
         */
        QHash<QString,ScriptingPlugin*> scriptingPlugins;
};

#endif // PLUGINMANAGERIMPL_H
