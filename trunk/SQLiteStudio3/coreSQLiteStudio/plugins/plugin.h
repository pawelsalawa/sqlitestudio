#ifndef PLUGIN_H
#define PLUGIN_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QHash>
#include <QtPlugin>

class PluginType;

/** @file */

/**
 * @brief General plugin interface.
 *
 * This is the top-most generic interface for SQLiteStudio plugins.
 * It's based in Qt's plugins framework. Every SQLiteStudio plugin must
 * implement this (or its descendant) interface.
 *
 * SQLiteStudio plugin is basicly class implementing this interface,
 * compiled as shared library (*.dll, *.so, *.dylib).
 *
 * Apart from implementing Plugin interface, the plugin class must also declare ::SQLITESTUDIO_PLUGIN macro, like this:
 * @code
 * class MyPlugin : Plugin
 * {
 *     Q_OBJECT
 *
 *     SQLITESTUDIO_PLUGIN
 *
 *     public:
 *         // ...
 * };
 * @endcode
 *
 * Full tutorial for writting plugins is at: http://sqlitestudio.pl/wiki/index.php/Writting_plugins
 *
 * SQLiteStudio looks for plugins in following directories:
 * <ul>
 * <li><tt>{current_executable_dir}/plugins</tt> - a "plugins" subdirectory of the directory where application binary is placed,</li>
 * <li><tt>{configuration_dir}/plugins</tt> - a "plugins" subdirectory of configuration directory detected and defined in Config,</li>
 * <li><tt>{env_var:SQLITESTUDIO_PLUGINS}</tt> - environment variable with name "SQLITESTUDIO_PLUGINS",</li>
 * <li><tt>{compile_time:PLUGINS_DIR}</tt> - compile time defined parameter's value of parameter with the name "PLUGINS_DIR".</li>
 * </ul>
 */
class API_EXPORT Plugin
{
    public:
        /**
         * @brief Virtual destructor to make sure all plugins are destroyed correctly.
         */
        virtual ~Plugin() {}

        /**
         * @brief Gets name of the plugin.
         * @return Name of the plugin.
         *
         * The name of the plugin is a kind of primary key for plugins. It has to be unique across all loaded plugins.
         * An attempt to load two plugins with the same name will result in failed load of the second plugin.
         *
         * The name is a kind of internal plugin's name. It's designated for presenting to the user
         * - for that purpose there is a getTitle().
         *
         * It's a good practice to keep it as single word. Providing plugin's class name can be a good idea.
         */
        virtual QString getName() const = 0;

        /**
         * @brief Gets title for the plugin.
         * @return Plugin title.
         *
         * This is plugin's name to be presented to the user. It can be multiple words name. It should be localized (translatable) text.
         * It's used solely for presenting plugin to the user, nothing more.
         */
        virtual QString getTitle() const = 0;

        /**
         * @brief Gets name of the configuration UI form.
         * @return Name of the form object.
         *
         * Some plugins may link (during compilation) only to the coreSQLiteStudio part of the application, but they can still
         * benefit from SQLiteStudio GUI application by providing UI form that will be used in ConfigDialog.
         *
         * This method should return the object name of the top-most widget found in the provided *.ui file.
         *
         * For more details see: http://sqlitestudio.pl/wiki/index.php/Plugin_UI_forms
         */
        virtual QString getConfigUiForm() const = 0;

        /**
         * @brief Provides name of the plugin's author.
         * @return Author name.
         *
         * This is displayed in ConfigDialog when user clicks on Details button of the plugin.
         */
        virtual QString getAuthor() const = 0;

        /**
         * @brief Provides some details on what does the plugin.
         * @return Plugin description.
         *
         * This is displayed in ConfigDialog when user clicks on Details button of the plugin.
         */
        virtual QString getDescription() const = 0;

        /**
         * @brief Provides plugin version number.
         * @return Version number.
         *
         * Version number format can be picked by plugin developer, but it is recommended
         * to use XXYYZZ, where XX is major version, YY is minor version and ZZ is patch version.
         * Of course the XX can be single X if major version is less then 10.
         *
         * This would result in versions like: 10000 (for version 1.0.0), or 10102 (for version 1.1.2),
         * or 123200 (for version 12.32.0).
         *
         * This is of course just a suggestion, you don't have to stick to it. Just keep in mind,
         * that this number is used by SQLiteStudio to compare plugin versions. If there's a plugin with higher version,
         * SQLiteStudio will propose to update it.
         *
         * The suggested format is also easier to convert to printable (string) version later in getPrintableVersion().
         */
        virtual int getVersion() const = 0;

        /**
         * @brief Provides formatted version string.
         * @return Version string.
         *
         * It provides string that represents version returned from getVersion() in a human-readable form.
         * It's a good practice to return versions like "1.3.2", or "1.5", as they are easy to read.
         *
         * This version string is presented to the user.
         */
        virtual QString getPrintableVersion() const = 0;

        /**
         * @brief Initializes plugin just after it was loaded.
         * @return true on success, or false otherwise.
         *
         * This is called as a first, just after plugin was loaded. If it returns false,
         * then plugin loading is considered to be failed and gets unloaded.
         *
         * If this method returns false, then deinit() is not called.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes plugin that is about to be unloaded.
         *
         * This is called just before plugin is unloaded. It's called only when plugin was loaded
         * successfully. It's NOT called when init() returned false.
         */
        virtual void deinit() = 0;
};

/**
 * @def SqliteStudioPluginInterface
 * @brief SQLiteStudio plugin interface ID.
 *
 * This is an ID string for Qt's plugins framework. It's used by ::SQLITESTUDIO_PLUGIN macro.
 * No need to use it directly.
 */
#define SqliteStudioPluginInterface "pl.sqlitestudio.Plugin/1.0"

/**
 * @def SQLITESTUDIO_PLUGIN
 * @brief Defines class as a SQLiteStudio plugin
 *
 * Every class implementing SQLiteStudio plugin must have this declaration,
 * otherwise SQLiteStudio won't be able to load the plugin.
 *
 * It has to be placed in class declaration:
 * @code
 * class MyPlugin : public QObject, public Plugin
 * {
 *     Q_OBJECT
 *     SQLITESTUDIO_PLUGIN
 *
 *     public:
 *         // ...
 * }
 * @endcode
 */
#define SQLITESTUDIO_PLUGIN(file)\
    Q_PLUGIN_METADATA(IID SqliteStudioPluginInterface FILE file) \
    Q_INTERFACES(Plugin)

Q_DECLARE_INTERFACE(Plugin, SqliteStudioPluginInterface)

#endif // PLUGIN_H
