#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "coreSQLiteStudio_global.h"
#include "plugin.h"
#include "plugintype.h"
#include <QPluginLoader>
#include <QStringList>
#include <QHash>
#include <QDebug>

class Plugin;

/** @file */

/**
 * @brief The plugin manager.
 *
 * It's a singleton accessible from SQLiteStudio class.
 *
 * It provides methods to load, unload and query plugins. It stores loaded
 * plugins in configuration on application close and loads that plugins during next startup.
 * If there's a plugin which was not defined if it was loaded or not - it is loaded by default.
 *
 * Description of Plugin interface contains list of directories scanned for plugins.
 *
 * There's a macro for global access to the PluginManager - ::PLUGINS. It actually calls
 * SQLiteStudio::getInstance() and from there it calls SQLiteStudio::getPluginManager().
 *
 * Plugins in PluginManager are organized by types. The manager has a list of types and for each type
 * there's a list of plugins of that type. Plugin types are represented by PluginType class.
 *
 * @section querying_plugins Querying available and loaded plugins
 *
 * To query all plugins available to the application (including those not loaded) use getAllPluginNames().
 *
 * To query if certain plugin is loaded use isLoaded().
 *
 * To query all plugins loaded to the application use getLoadedPlugins(). It requires either PluginType,
 * or plugin interface class (for template method version) to determinate what group of plugins you're
 * interested in. To return all plugins (no matter what type), use template method version with Plugin
 * as an interface type for parameter. An example of getting all SQL formatter plugins:
 * @code
 * QList<SqlFormatterPlugin*> formatterPlugins = PLUGINS->getLoadedPlugins<SqlFormatterPlugin>();
 * @endcode
 *
 * To get list of plugin types use getPluginTypes().
 *
 * To get PluginType for given plugin interface use getPluginType<PluginInterfaceClass>().
 *
 * These are just the most important methods to query plugins. See full list of methods for more.
 *
 * @section load_unload Loading and unloading plugins
 *
 * To load plugin use load().
 *
 * To unload plugin use unload().
 *
 * Apart from that, all plugins are loaded initially (unless they were unloaded last time during
 * application close).
 *
 * @section plugin_types Specialized plugin types
 *
 * Each plugin must implement Plugin interface, but it also can implement other interfaces,
 * which makes them suitable for fulfilling certain functionalities. For example all plugins
 * implementing SqlFormatterPlugin will automatically be available to SqlFormatter object,
 * because PluginManager knows which plugins implement SqlFormatterPlugin and can provide full
 * list of those plugins to SqlFormatter. This is done by call to registerPluginType().
 *
 * The registerPluginType() registers new type of plugins that will be recognizable by PluginManager.
 * Once the new interface is registered with this method, all plugins will be tested against
 * implementation for that type and those which implement the interface will be stored
 * in the proper collection assigned for that plugin type.
 *
 * This way PluginManager can provide list of all plugins implementing given interface
 * with getLoadedPlugins().
 *
 * All registered plugin types can be queries by getPluginTypes() method.
 */
class API_EXPORT PluginManager : public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Creates plugin manager.
         */
        PluginManager();

        /**
         * @brief Deletes plugin manager.
         */
        ~PluginManager();

        /**
         * @brief Loads all plugins.
         *
         * Scans all plugin directories and tries to load all plugins found there. For list of directories
         * see description of Plugin class.
         */
        void init();

        /**
         * @brief Unloads all loaded plugins.
         *
         * Also deregisters all plugin types.
         */
        void deinit();

        /**
         * @brief Provides list of registered plugin types.
         * @return List of registered plugin types.
         */
        QList<PluginType*> getPluginTypes() const;

        /**
         * @brief Provides list of plugin directories.
         * @return List of directory paths (not necessarily absolute paths).
         */
        QStringList getPluginDirs() const;

        /**
         * @brief Provides absolute path to the plugin's file.
         * @param plugin Loaded plugin.
         * @return Absolute path to the plugin file.
         */
        QString getFilePath(Plugin* plugin) const;

        /**
         * @brief Loads instance of built-in plugin into the manager.
         * @param plugin Plugin instance.
         * @return true on success or false on failure (plugin's type could not be matched to registered plugin types).
         *
         * Built-in plugins are classes that implement plugin interface, but they are not in separate library.
         * Instead they are classes compiled and linked to the main application. Such classes should be instantiated
         * and passed to this method, so the PluginManager can treat it as any other plugin.
         *
         * @note Built-in plugins cannot be loaded or unloaded, so calls to load() or unload() will make no effect.
         */
        bool loadBuiltInPlugin(Plugin* plugin);

        /**
         * @brief Loads the plugin.
         * @param pluginName Name of the plugin to load.
         * @return true on success, or false on failure.
         *
         * When loading a plugin, PluginManager loads the plugin file and resolves all its symbols inside.
         * If that failed, file gets unloaded and the method returns false.
         *
         * Qt plugins framework will require that the loaded plugin will provide exactly one Plugin interface
         * implementation. Otherwise file will be unloaded and this method will return false.
         *
         * Then the Plugin::init() method is called. It it returns false, then plugin is unloaded
         * and this method returns false.
         *
         * Then meta information is read from the plugin (title, version, author, etc) - see Plugin for details.
         *
         * Then loaded plugin passes several tests against all registered plugin types. If it implements
         * any type, it's added to the plugin list of that type.
         *
         * Then the loaded() signal is emitted. Finally, the true value is returned.
         */
        bool load(const QString& pluginName);

        /**
         * @brief Unloads plugin.
         * @param pluginName Plugin name to be unloaded.
         *
         * If the plugin is not loaded, this method does nothing.
         * First the aboutToUnload() signal is emitted. Then Plugin::deinit() is called.
         * Then the plugin library is unloaded (which causes Qt's plugins framework to delete the object
         * implementing Plugin interface before the actual unloading).
         *
         * Finally, the unloaded() signal is emitted.
         */
        void unload(const QString& pluginName);

        /**
         * @brief Unloads plugin.
         * @param plugin Loaded plugin to be unloaded.
         * @overload void unload(Plugin* plugin)
         */
        void unload(Plugin* plugin);

        /**
         * @brief Tests if given plugin is loaded.
         * @param pluginName Name of the plugin to test.
         * @return true if the plugin is loaded, or false otherwise.
         */
        bool isLoaded(const QString& pluginName) const;

        /**
         * @brief Tests whether given plugin is one of built-in plugins.
         * @param pluginName Name of the plugin to test.
         * @return true if the plugin is the built-in one, or false otherwise.
         *
         * @see loadBuiltInPlugin()
         */
        bool isBuiltIn(const QString& pluginName) const;

        /**
         * @brief Finds loaded plugin by name.
         * @param pluginName Plugin name to look for.
         * @return Loaded plugin object, or null of the plugin is not loaded.
         */
        Plugin* getLoadedPlugin(const QString& pluginName) const;

        /**
         * @brief Provides list of plugin names of given type.
         * @param type Type of plugins to get names for.
         * @return List of names.
         *
         * It returns names for all plugins available for the application,
         * no matter they're currently loaded or not.
         */
        QStringList getAllPluginNames(PluginType* type) const;

        /**
         * @brief Provides list of all plugin names.
         * @return All available plugin names, no matter if loaded or not.
         */
        QStringList getAllPluginNames() const;

        /**
         * @brief Finds plugin's type.
         * @param pluginName Plugin name (can be unloaded plugin).
         * @return Type of the plugin, or null if plugin was not found by the name.
         */
        PluginType* getPluginType(const QString& pluginName) const;

        /**
         * @brief Provides plugin's author.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Author string defined in the plugin.
         */
        QString getAuthor(const QString& pluginName) const;

        /**
         * @brief Provides plugin's title.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Title string defined in the plugin.
         */
        QString getTitle(const QString& pluginName) const;

        /**
         * @brief Provides human-readable version of the plugin.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Version string defined in the plugin.
         */
        QString getPrintableVersion(const QString& pluginName) const;

        /**
         * @brief Provides numeric version of the plugin.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Numeric version defined in the plugin.
         */
        int getVersion(const QString& pluginName) const;

        /**
         * @brief Provides detailed description about the plugin.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Description defined in the plugin.
         */
        QString getDescription(const QString& pluginName) const;

        /**
         * @brief registerPluginType Registers plugin type for loading and managing.
         * @tparam T Interface class (as defined by Qt plugins standard)
         * @param form Optional name of form object.
         * @param title Optional title for configuration dialog.
         * The form object name is different if you register new type by general type plugin.
         * Built-in types are defined as the name of page from ConfigDialog.
         * Types registered from plugins should use top widget name defined in the ui file.
         * The title parameter is required if the configuration form was defined outside (in plugin).
         * Title will be used for configuration dialog to display plugin type category (on the left of the dialog).
         */
        template <class T>
        void registerPluginType(const QString& title, const QString& form = QString())
        {
            PluginType* type = new DefinedPluginType<T>(title, form);
            registeredPluginTypes << type;
        }

        /**
         * @brief Tells plugin's type.
         * @param plugin Loaded plugin.
         * @return Type of the plugin.
         */
        PluginType* getPluginType(Plugin* plugin) const;

        /**
         * @brief Gets plugin type for given plugin interface.
         * @tparam T Interface class of the plugin.
         * @return Type of the plugin for given interface if registered, or null otherwise.
         */
        template <class T>
        PluginType* getPluginType() const
        {
            foreach (PluginType* type, registeredPluginTypes)
            {
                if (!dynamic_cast<DefinedPluginType<T>*>(type))
                    continue;

                return type;
            }
            return nullptr;
        }

        /**
         * @brief Provides list of plugins for given plugin type.
         * @param type Type of plugins.
         * @return List of plugins for given type.
         *
         * This version of the method takes plugin type object as an discriminator.
         * This way you can iterate through all types (using getPluginTypes())
         * and then for each type get list of plugins for that type, using this method.
         */
        QList<Plugin*> getLoadedPlugins(PluginType* type) const;

        /**
         * @brief Provide list of plugins of given type.
         * @tparam T Interface class of plugins, that we want to get.
         *
         * This method version gets plugin interface type as template parameter,
         * so it returns list of loaded plugins that are already casted to requested
         * interface type.
         */
        template <class T>
        QList<T*> getLoadedPlugins() const
        {
            QList<T*> typedPlugins;
            PluginType* type = getPluginType<T>();
            if (!type)
                return typedPlugins;

            foreach (PluginContainer* container, pluginCategories[type])
            {
                if (container->loaded)
                    typedPlugins << dynamic_cast<T*>(container->plugin);
            }

            return typedPlugins;
        }

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

    signals:
        /**
         * @brief Emitted just before plugin is unloaded.
         * @param plugin Plugin object to be unloaded.
         * @param type Type of the plugin.
         *
         * It's emitted just before call to Plugin::deinit(), destroying plugin object
         * and unloading the plugin file.
         *
         * Any code using certain plugin should listen for this signal and stop using
         * the plugin immediately when received this signal. Otherwise application may crash.
         */
        void aboutToUnload(Plugin* plugin, PluginType* type);

        /**
         * @brief Emitted just after plugin was loaded.
         * @param plugin Plugin object from loaded plugin.
         * @param type Plugin type.
         *
         * It's emitted after plugin was loaded and successfully initialized (which includes
         * successful Plugin::init() call).
         */
        void loaded(Plugin* plugin, PluginType* type);

        /**
         * @brief Emitted after plugin was unloaded.
         * @param pluginName Name of the plugin that was unloaded.
         * @param type Type of the plugin.
         *
         * Emitted after plugin was deinitialized and unloaded. At this stage a plugin object
         * is no longer available, only it's name and other metadata (like description, version, etc).
         */
        void unloaded(const QString& pluginName, PluginType* type);

        /**
         * @brief Emitted after initial plugin set was loaded.
         *
         * The initial load is performed at application startup. Any code that relies on
         * some plugins being loaded (like for example code that loads list of databases relies on
         * database support plugins) should listen to this signal.
         */
        void pluginsInitiallyLoaded();
};

/**
 * @def PLUGINS
 * @brief PluginsManager instance access macro.
 *
 * Since SQLiteStudio creates only one instance of PluginsManager,
 * there is a standard method for accessing it, using code:
 * @code
 * QList<PluginType*> types = SQLiteStudio::getInstance()->getPluginManager()->getPluginTypes();
 * @endcode
 * or there is simplified method, using this macro:
 * @code
 * QList<PluginType*> types = PLUGINS->getPluginTypes();
 * @endcode
 */
#define PLUGINS SQLiteStudio::getInstance()->getPluginManager()

#endif // PLUGINMANAGER_H
