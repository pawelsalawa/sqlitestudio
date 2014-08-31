#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "coreSQLiteStudio_global.h"
#include "plugins/plugin.h"
#include "plugins/plugintype.h"
#include "common/global.h"
#include "sqlitestudio.h"
#include <QStringList>

class Plugin;
class ScriptingPlugin;

/** @file */

/**
 * @brief The plugin manager.
 *
 * It's a singleton accessible with PLUGINS macro.
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
        struct PluginDetails
        {
            QString name;
            QString title;
            QString description;
            bool builtIn = false;
            int version = 0;
            QString versionString;
        };

        /**
         * @brief Loads all plugins.
         *
         * Scans all plugin directories and tries to load all plugins found there. For list of directories
         * see description of Plugin class.
         */
        virtual void init() = 0;

        /**
         * @brief Unloads all loaded plugins.
         *
         * Also deregisters all plugin types.
         */
        virtual void deinit() = 0;

        /**
         * @brief Provides list of registered plugin types.
         * @return List of registered plugin types.
         */
        virtual QList<PluginType*> getPluginTypes() const = 0;

        /**
         * @brief Provides list of plugin directories.
         * @return List of directory paths (not necessarily absolute paths).
         */
        virtual QStringList getPluginDirs() const = 0;

        /**
         * @brief Provides absolute path to the plugin's file.
         * @param plugin Loaded plugin.
         * @return Absolute path to the plugin file.
         */
        virtual QString getFilePath(Plugin* plugin) const = 0;

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
        virtual bool loadBuiltInPlugin(Plugin* plugin) = 0;

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
        virtual bool load(const QString& pluginName) = 0;

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
        virtual void unload(const QString& pluginName) = 0;

        /**
         * @brief Unloads plugin.
         * @param plugin Loaded plugin to be unloaded.
         * @overload
         */
        virtual void unload(Plugin* plugin) = 0;

        /**
         * @brief Tests if given plugin is loaded.
         * @param pluginName Name of the plugin to test.
         * @return true if the plugin is loaded, or false otherwise.
         */
        virtual bool isLoaded(const QString& pluginName) const = 0;

        /**
         * @brief Tests whether given plugin is one of built-in plugins.
         * @param pluginName Name of the plugin to test.
         * @return true if the plugin is the built-in one, or false otherwise.
         *
         * @see loadBuiltInPlugin()
         */
        virtual bool isBuiltIn(const QString& pluginName) const = 0;

        /**
         * @brief Finds loaded plugin by name.
         * @param pluginName Plugin name to look for.
         * @return Loaded plugin object, or null of the plugin is not loaded.
         */
        virtual Plugin* getLoadedPlugin(const QString& pluginName) const = 0;

        /**
         * @brief Provides list of plugin names of given type.
         * @param type Type of plugins to get names for.
         * @return List of names.
         *
         * It returns names for all plugins available for the application,
         * no matter they're currently loaded or not.
         */
        virtual QStringList getAllPluginNames(PluginType* type) const = 0;

        virtual QList<PluginDetails> getAllPluginDetails() const = 0;
        virtual QList<PluginDetails> getLoadedPluginDetails() const = 0;

        /**
         * @brief Provides list of all plugin names.
         * @return All available plugin names, no matter if loaded or not.
         */
        virtual QStringList getAllPluginNames() const = 0;

        /**
         * @brief Finds plugin's type.
         * @param pluginName Plugin name (can be unloaded plugin).
         * @return Type of the plugin, or null if plugin was not found by the name.
         */
        virtual PluginType* getPluginType(const QString& pluginName) const = 0;

        /**
         * @brief Provides plugin's author.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Author string defined in the plugin.
         */
        virtual QString getAuthor(const QString& pluginName) const = 0;

        /**
         * @brief Provides plugin's title.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Title string defined in the plugin.
         */
        virtual QString getTitle(const QString& pluginName) const = 0;

        /**
         * @brief Provides human-readable version of the plugin.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Version string defined in the plugin.
         */
        virtual QString getPrintableVersion(const QString& pluginName) const = 0;

        /**
         * @brief Provides numeric version of the plugin.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Numeric version defined in the plugin.
         */
        virtual int getVersion(const QString& pluginName) const = 0;

        /**
         * @brief Provides detailed description about the plugin.
         * @param pluginName Name of the plugin (can be unloaded plugin).
         * @return Description defined in the plugin.
         */
        virtual QString getDescription(const QString& pluginName) const = 0;

        /**
         * @brief Tells plugin's type.
         * @param plugin Loaded plugin.
         * @return Type of the plugin.
         */
        virtual PluginType* getPluginType(Plugin* plugin) const = 0;

        /**
         * @brief Provides list of plugins for given plugin type.
         * @param type Type of plugins.
         * @return List of plugins for given type.
         *
         * This version of the method takes plugin type object as an discriminator.
         * This way you can iterate through all types (using getPluginTypes())
         * and then for each type get list of plugins for that type, using this method.
         */
        virtual QList<Plugin*> getLoadedPlugins(PluginType* type) const = 0;

        /**
         * @brief Provides list of all loaded plugins.
         * @return List of plugins.
         */
        virtual QList<Plugin*> getLoadedPlugins() const = 0;

        /**
         * @brief Provides names of all loaded plugins.
         * @return List of plugin names.
         */
        virtual QStringList getLoadedPluginNames() const = 0;

        /**
         * @brief Provides scripting plugin for given scripting language if available.
         * @param languageName Scripting language name to get plugin for.
         * @return Plugin object or null if proper plugin was not found.
         *
         * Calling this function is similar in results to call to getLoadedPlugins<ScriptingPlugin>()
         * and then extracting a single plugin with desired scripting language support, except
         * calling this function is much faster. PluginManager keeps scripting language plugins
         * internally in hash table with language names as keys, so getting scripting plugin
         * for desired language is way faster when using this method.
         */
        virtual ScriptingPlugin* getScriptingPlugin(const QString& languageName) const = 0;

        /**
         * @brief Loads metadata from given Json object.
         * @param The metadata from json file.
         * @return Metadata with keys: type, name, title, description, version, author, ui (optional).
         */
        virtual QHash<QString,QVariant> readMetaData(const QJsonObject& metaData) = 0;

        /**
         * @brief Converts integer version to string version.
         * @param version Integer version in XXYYZZ standard (see Plugin::getVersion() for details).
         * @return Printable version string.
         */
        virtual QString toPrintableVersion(int version) const = 0;

        /**
         * @brief Provides list of plugin names that the queried plugin depends on.
         * @param pluginName Queried plugin name.
         * @return List of plugin names, usually an empty list.
         *
         * This is the list that is declared in plugins metadata under the "dependencies" key.
         * The plugin can be loaded only if all its dependencies were successfully loaded.
         */
        virtual QStringList getDependencies(const QString& pluginName) const = 0;

        /**
         * @brief Provides list of plugin names that are declared to be in conflict with queries plugin.
         * @param pluginName Queried plugin name,
         * @return List of plugin names, usually an empty list.
         *
         * If a plugin declares other plugin (by name) to be its conflict (a "conflicts" key in plugin's metadata),
         * then those 2 plugins cannot be loaded at the same time. SQLiteStudio will always refuse to load
         * the other one, if the first one is already loaded - and vice versa.
         *
         * Declaring conflicts for a plugin can be useful for example if somebody wants to proivde an alternative
         * implementation of SQLite2 database plugin, etc. In that case SQLiteStudio won't get confused in
         * deciding which plugin to use for supporting such databases.
         */
        virtual QStringList getConflicts(const QString& pluginName) const = 0;

        /**
         * @brief Tells if plugins were already loaded on startup, or is this yet to happen.
         * @return true if plugins were loaded, false if they are going to be loaded.
         */
        virtual bool arePluginsInitiallyLoaded() const = 0;

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
            registerPluginType(new DefinedPluginType<T>(title, form));
        }

        /**
         * @brief Gets plugin type for given plugin interface.
         * @tparam T Interface class of the plugin.
         * @return Type of the plugin for given interface if registered, or null otherwise.
         */
        template <class T>
        PluginType* getPluginType() const
        {
            foreach (PluginType* type, getPluginTypes())
            {
                if (!dynamic_cast<DefinedPluginType<T>*>(type))
                    continue;

                return type;
            }
            return nullptr;
        }

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

            foreach (Plugin* plugin, getLoadedPlugins(type))
                typedPlugins << dynamic_cast<T*>(plugin);

            return typedPlugins;
        }

        /**
         * @brief Provide list of plugin names of given type.
         * @tparam T Interface class of plugins, that we want to get names for.
         *
         * This method version gets plugin interface type as template parameter,
         * so it returns list of names of loaded plugins.
         */
        template <class T>
        QStringList getLoadedPluginNames() const
        {
            QStringList names;
            PluginType* type = getPluginType<T>();
            if (!type)
                return names;

            foreach (Plugin* plugin, getLoadedPlugins(type))
                names << plugin->getName();

            return names;
        }

    protected:
        /**
         * @brief Adds given type to registered plugins list.
         * @param type Type instance.
         *
         * This is a helper method for registerPluginType<T>() template function.
         * The implementation should register given plugin type, that is - add it to a list of registered types.
         */
        virtual void registerPluginType(PluginType* type) = 0;

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

        /**
         * @brief Emitted when the plugin manager is deinitializing and will unload all plugins in a moment.
         *
         * It's emitted when user closes application, so the plugin manager deinitializes and unloads all plugins.
         * This signal is emitted just before plugins get unloaded.
         * If some signal handler is not interested in mass plugin unloading, then it can handle this signal
         * and disconnect from unloaded() signal.
         */
        void aboutToQuit();

        /**
         * @brief Emitted when plugin load was requested, but it failed.
         * @param pluginName Name of the plugin that failed to load.
         *
         * It's used for example by ConfigDialog to uncheck plugin that was requested to load (checked) and it failed.
         */
        void failedToLoad(const QString& pluginName);
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
 * or there's a slightly simpler way:
 * @code
 * QList<PluginType*> types = SQLITESTUDIO->getPluginManager()->getPluginTypes();
 * @endcode
 * or there is a very simplified method, using this macro:
 * @code
 * QList<PluginType*> types = PLUGINS->getPluginTypes();
 * @endcode
 */
#define PLUGINS SQLITESTUDIO->getPluginManager()

#endif // PLUGINMANAGER_H
