#ifndef PLUGINMANAGERIMPL_H
#define PLUGINMANAGERIMPL_H

#include "services/pluginmanager.h"
#include <QPluginLoader>
#include <QHash>

class API_EXPORT PluginManagerImpl : public PluginManager
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
        QHash<QString,QVariant> readMetaData(const QJsonObject& metaData);
        QString toPrintableVersion(int version) const;
        QStringList getDependencies(const QString& pluginName) const;
        QStringList getConflicts(const QString& pluginName) const;
        bool arePluginsInitiallyLoaded() const;
        QList<Plugin*> getLoadedPlugins() const;
        QStringList getLoadedPluginNames() const;
        QList<PluginDetails> getAllPluginDetails() const;
        QList<PluginDetails> getLoadedPluginDetails() const;

    protected:
        void registerPluginType(PluginType* type);

    private:
        struct PluginDependency
        {
            QString name;
            int minVersion = 0;
            int maxVersion = 0;
        };

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
            PluginType* type = nullptr;

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
            QPluginLoader* loader = nullptr;

            /**
             * @brief Plugin object.
             *
             * It's null when plugin is not loaded.
             */
            Plugin* plugin = nullptr;

            /**
             * @brief Flag indicating that the plugin is built in.
             *
             * Plugins built-in are classes implementing plugin's interface,
             * but they are compiled and statically linked to the main application binary.
             * They cannot be loaded or unloaded - they are loaded by default.
             */
            bool builtIn = false;

            /**
             * @brief Flag indicating that plugin should be loaded, unless user unloaded it manually.
             *
             * If this flag is set to false, then the plugin will not be loaded, even it was not manually unloaded.
             * This flag can be defined in plugin's json file using property named 'loadByDefault'.
             */
            bool loadByDefault = true;

            /**
             * @brief Names of plugnis that this plugin depends on.
             */
            QList<PluginDependency> dependencies;

            /**
             * @brief Names of plugins that this plugin conflicts with.
             */
            QStringList conflicts;

            /**
             * @brief If not empty, contains Plugin's project name to be used for loading translation resource file.
             *
             * For typical SQLiteStudio plugin the auto-generated translation resource name is the same
             * as the name of the plugin project. Typically, name of loaded plugin class is made of
             * the name of the plugin project and the "Plugin" word suffix. Therefore SQLiteStudio
             * by default just removes the "Plugin" suffix (if it has such) and attempts to load the translation
             * named this way.
             *
             * If the main Plugin class does not follow this naming strategy (project name + Plugin suffix),
             * then the translationName should be specified in plugin's metadata,
             * giving actual name of translation resource (i.e. name of Plugin's source code project) to be loaded.
             */
            QString translationName;
        };

        /**
         * @brief List of plugins, both loaded and unloaded.
         */
        typedef QList<PluginContainer*> PluginContainerList;

        /**
         * @brief Scans plugin directories to find out available plugins.
         *
         * It looks in the following locations:
         * <ul>
         * <li> application_directory/plugins/
         * <li> application_config_directory/plugins/
         * <li> directory pointed by the SQLITESTUDIO_PLUGINS environment variable
         * <li> directory compiled in as PLUGINS_DIR parameter of the compilation
         * </ul>
         *
         * The application_directory is a directory where the application executable is.
         * The application_config_directory can be different, see ConfigImpl::initDbFile() for details.
         * The SQLITESTUDIO_PLUGINS variable can contain several paths, separated by : (for Unix/Mac) or ; (for Windows).
         */
        void scanPlugins();

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
         * @brief Loads given plugin.
         * @param pluginName Name of the plugin to load.
         * @param alreadyAttempted List of plugin names that were already attempted to be loaded.
         * @param minVersion Minimum required version of the plugin to load.
         * @param maxVersion Maximum required version of the plugin to load.
         * @return true on success, false on failure.
         *
         * This is pretty much what the public load() method does, except this one tracks what plugins were already
         * attempted to be loaded (and failed), so it doesn't warn twice about the same plugin if it failed
         * to load while it was a dependency for some other plugins.
         *
         * It also allows to define minimum and maximum plugin version, so if SQLiteStudio has the plugin available,
         * but the version is out of required range, it will also fail to load.
         */
        bool load(const QString& pluginName, QStringList& alreadyAttempted, int minVersion = 0, int maxVersion = 0);

        /**
         * @brief Executes standard routines after plugin was loaded.
         * @param container Container for the loaded plugin.
         *
         * It fills all members of the plugin container and emits loaded() signal.
         */
        void pluginLoaded(PluginContainer* container);

        /**
         * @brief Stores some specific plugin types in internal collections for faster access.
         * @param plugin Plugin that was just loaded.
         *
         * This is called after we are sure we have a Plugin instance.
         *
         * The method stores certain plugin types in internal collections, so they can be accessed
         * faster, instead of calling getLoadedPlugin<T>(), which is not as fast.
         *
         * The internal collections are used for plugins that are likely to be accessed frequently,
         * like ScriptingPlugin.
         */
        void addPluginToCollections(Plugin* plugin);

        /**
         * @brief Removes plugin from internal collections.
         * @param plugin Plugin that is about to be unloaded.
         *
         * This is the reverse operation to what addPluginToCollections(Plugin*) does.
         */
        void removePluginFromCollections(Plugin* plugin);

        /**
         * @brief Reads title, description, author, etc. from the plugin.
         * @param plugin Plugin to read data from.
         * @param container Container to put the data to.
         * @return true on success, false on problems (with details in logs)
         *
         * It does the reading by calling all related methods from Plugin interface,
         * then stores those information in given \p container.
         *
         * The built-in plugins define those methods using their class metadata.
         *
         * External plugins provide this information in their file metadata
         * and this method uses QPluginLoader to read this metadata.
         */
        bool readMetaData(PluginContainer* container);

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
        bool initPlugin(QPluginLoader* loader, const QString& fileName);

        bool checkPluginRequirements(const QString& pluginName, const QJsonObject& metaObject);
        bool readDependencies(const QString& pluginName, PluginContainer* container, const QJsonValue& depsValue);
        bool readConflicts(const QString& pluginName, PluginContainer* container, const QJsonValue& confValue);

        /**
         * @brief Creates plugin container and initializes it.
         * @param plugin Built-in plugin object.
         * @return true if the initialization succeeded, or false otherwise.
         *
         * This is pretty much the same as the other initPlugin() method, but this one is for built-in plugins.
         */
        bool initPlugin(Plugin* plugin);

        /**
         * @brief Tests if given plugin is configured to be loaded at startup.
         * @param plugin Tested plugin object.
         * @return true if plugin should be loaded at startup, or false otherwise.
         *
         * This method checks General.LoadedPlugins configuration entry to see if plugin
         * was explicitly disabled for loading at startup.
         */
        bool shouldAutoLoad(const QString& pluginName);

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
         * Keys are scripting language name. It's a separate table to optimize querying scripting plugins.
         */
        QHash<QString,ScriptingPlugin*> scriptingPlugins;

        bool pluginsAreInitiallyLoaded = false;
};

#endif // PLUGINMANAGERIMPL_H
