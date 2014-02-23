#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "plugin.h"
#include <QObject>
#include <QtPlugin>

/** @file */

/**
 * @brief Helper class for implementing plugins
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
 */
class API_EXPORT GenericPlugin : public virtual QObject, public virtual Plugin
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
         * @return Title defined with ::SQLITESTUDIO_PLUGIN_TITLE or (if not defined) the same value as getName().
         */
        QString getTitle() const;

        /**
         * @brief Provides UI form name.
         * @return Form name defined by ::SQLITESTUDIO_PLUGIN_UI, or null QString if not defined.
         */
        QString getConfigUiForm() const;

        /**
         * @brief Provides plugin description.
         * @return Description as defined with ::SQLITESTUDIO_PLUGIN_DESC, or null QString if not defined.
         */
        QString getDescription() const;

        /**
         * @brief Provides plugin numeric version.
         * @return Version number as defined with ::SQLITESTUDIO_PLUGIN_VERSION, or 0 if not defined.
         */
        int getVersion() const;

        /**
         * @brief Converts plugin version to human readable form.
         * @return Version in format X.Y.Z.
         */
        QString getPrintableVersion() const;

        /**
         * @brief Provides an author name.
         * @return Author name as defined with ::SQLITESTUDIO_PLUGIN_AUTHOR, or null QString if not defined.
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
 * Plugin title is used on the user interface as a name of the plugin.
 *
 * This is optional parameter of the plugin, but it's good practice to define it, because if it's not defined,
 * then plugin's class name will be used for the title and it may be confusing to the end user.
 */
#define SQLITESTUDIO_PLUGIN_TITLE(Title) Q_CLASSINFO("title", Title)

/**
 * @def SQLITESTUDIO_PLUGIN_DESC
 * @brief Defines plugin description.
 *
 * Description is presented to the user when he clicks "Details" for the plugin in configuration dialog.
 * It should describe some details about what the plugin adds to the application, or how it works.
 *
 * It is optional parameter of the plugin.
 */
#define SQLITESTUDIO_PLUGIN_DESC(Desc) Q_CLASSINFO("description", Desc)

/**
 * @def SQLITESTUDIO_PLUGIN_UI
 * @brief Defines Qt Designer Form object name to be used in configuration dialog.
 *
 * The name defined by this parameter will be used in configuration dialog to let user configure
 * this plugin. The top-most object of the form must have the same name as the name defined by this parameter
 * and it has to be unique across forms provided by all plugins.
 *
 * It is optional parameter of the plugin.
 *
 * TODO reference to wiki about creating plugin forms
 */
#define SQLITESTUDIO_PLUGIN_UI(FormName) Q_CLASSINFO("ui", FormName)

/**
 * @def SQLITESTUDIO_PLUGIN_VERSION
 * @brief Defines plugin version.
 *
 * Version should be provided in form of an integer, for example:
 * @code
 * SQLITESTUDIO_PLUGIN_VERSION(20005)
 * @endcode
 *
 * It should be in format XXYYZZ, where XX is major version, YY is minor version and ZZ is patch version.
 * If XX is less then 10, then it can be a single digit.
 *
 * For example 30521 means that plugin version is 3.5.21.
 *
 * This is mandatory parameter of the plugin. When ommited, then 0 is used by default, which may lead
 * to problems later on (when new version is release with still no version defined).
 */
#define SQLITESTUDIO_PLUGIN_VERSION(Ver) Q_CLASSINFO("version", #Ver)

/**
 * @def SQLITESTUDIO_PLUGIN_AUTHOR
 * @brief Defines an author of the plugin.
 *
 * Author can be either full name, a nickname, an organization name, or anything else. It's used purely
 * for the information to the user when he clicks "Details" for the plugin in configuration dialog.
 *
 * It is optional parameter of the plugin.
 */
#define SQLITESTUDIO_PLUGIN_AUTHOR(Author) Q_CLASSINFO("author", Author)

#endif // GENERICPLUGIN_H
