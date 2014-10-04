#ifndef BUILTINPLUGIN_H
#define BUILTINPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "plugins/plugin.h"

/**
 * @brief Helper class for implementing built-in plugins
 *
 * This class can be inherited, so most of the abstract methods from Plugin interface get implemented.
 * All details (description, name, title, author, ...) are defined using Q_CLASSINFO.
 *
 * There are macros defined to help you with defining those details. You don't need to define
 * Q_CLASSINFO and know all required keys. You can use following macros:
 * <ul>
 * <li>::SQLITESTUDIO_PLUGIN_TITLE</li>
 * <li>::SQLITESTUDIO_PLUGIN_DESC</li>
 * <li>::SQLITESTUDIO_PLUGIN_UI</li>
 * <li>::SQLITESTUDIO_PLUGIN_VERSION</li>
 * <li>::SQLITESTUDIO_PLUGIN_AUTHOR</li>
 * </ul>
 *
 * Most of plugin implementations will use this class as a base, because it simplifies process
 * of plugin development. Using this class you don't have to implement any of virtual methods
 * from Plugin interface. It's enough to define meta information, like this:
 * @code
 * class MyPlugin : GenericPlugin
 * {
 *     Q_OBJECT
 *
 *     SQLITESTUDIO_PLUGIN
 *     SQLITESTUDIO_PLUGIN_TITLE("My plugin")
 *     SQLITESTUDIO_PLUGIN_DESC("Does nothing. It's an example plugin.")
 *     SQLITESTUDIO_PLUGIN_UI("formObjectName")
 *     SQLITESTUDIO_PLUGIN_VERSION(10000)
 *     SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
 * };
 * @endcode
 */class API_EXPORT BuiltInPlugin : public QObject, public virtual Plugin
{
        Q_OBJECT
        Q_INTERFACES(Plugin)

    public:
        /**
         * @brief Provides plugin internal name.
         * @return Plugin class name.
         */
        QString getName() const;

        /**
         * @brief Provides plugin title.
         * @return Title defined in plugin's metadata file with key "title" or (if not defined) the same value as getName().
         */
        QString getTitle() const;

        /**
         * @brief Provides plugin description.
         * @return Description as defined in plugin's metadata file with key "description", or null QString if not defined.
         */
        QString getDescription() const;

        /**
         * @brief Provides plugin numeric version.
         * @return Version number as defined in plugin's metadata file with key "version", or 0 if not defined.
         */
        int getVersion() const;

        /**
         * @brief Converts plugin version to human readable form.
         * @return Version in format X.Y.Z.
         */
        QString getPrintableVersion() const;

        /**
         * @brief Provides an author name.
         * @return Author name as defined with in plugin's metadata file with key "author", or null QString if not defined.
         */
        QString getAuthor() const;

        /**
         * @brief Does nothing.
         * @return Always true.
         *
         * This is a default (empty) implementation of init() for plugins.
         */
        bool init();

        /**
         * @brief Does nothing.
         *
         * This is a default (empty) implementation of init() for plugins.
         */
        void deinit();

    private:
        /**
         * @brief Extracts class meta information with given key.
         * @param key Key to extract.
         * @return Value of the meta information, or null if there's no information with given key.
         *
         * This is a helper method which queries Qt's meta object subsystem for class meta information defined with Q_CLASSINFO.
         */
        const char* getMetaInfo(const QString& key) const;
};

/**
 * @def SQLITESTUDIO_PLUGIN_TITLE
 * @brief Defines plugin title.
 *
 * This is a built-in plugin replacement for "title" key in external plugin's json metadata file.
 */
#define SQLITESTUDIO_PLUGIN_TITLE(Title) Q_CLASSINFO("title", Title)

/**
 * @def SQLITESTUDIO_PLUGIN_DESC
 * @brief Defines plugin description.
 *
 * This is a built-in plugin replacement for "description" key in external plugin's json metadata file.
 */
#define SQLITESTUDIO_PLUGIN_DESC(Desc) Q_CLASSINFO("description", Desc)

/**
 * @def SQLITESTUDIO_PLUGIN_UI
 * @brief Defines Qt Designer Form object name to be used in configuration dialog.
 *
 * This is a built-in plugin replacement for "ui" key in external plugin's json metadata file.
 */
#define SQLITESTUDIO_PLUGIN_UI(FormName) Q_CLASSINFO("ui", FormName)

/**
 * @def SQLITESTUDIO_PLUGIN_VERSION
 * @brief Defines plugin version.
 *
 * This is a built-in plugin replacement for "version" key in external plugin's json metadata file.
 */
#define SQLITESTUDIO_PLUGIN_VERSION(Ver) Q_CLASSINFO("version", #Ver)

/**
 * @def SQLITESTUDIO_PLUGIN_AUTHOR
 * @brief Defines an author of the plugin.
 *
 * This is a built-in plugin replacement for "author" key in external plugin's json metadata file.
 */
#define SQLITESTUDIO_PLUGIN_AUTHOR(Author) Q_CLASSINFO("author", Author)

#endif // BUILTINPLUGIN_H
