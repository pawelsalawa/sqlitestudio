#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "plugin.h"
#include <QObject>
#include <QtPlugin>
#include <QHash>
#include <QVariant>

/** @file */

/**
 * @brief Helper class for implementing plugins
 *
 * This class can be inherited, so most of the abstract methods from Plugin interface get implemented.
 * All details (description, name, title, author, ...) are defined in separate json file.
 *
 * Most of plugin implementations will use this class as a base, because it simplifies process
 * of plugin development. Using this class you don't have to implement any of virtual methods
 * from Plugin interface. It's enough to define meta information in the json file, like this:
 * @code
 * {
 *     "type":        "ScriptingPlugin",
 *     "title":       "My plugin",
 *     "description": "Does nothing. It's an example plugin.",
 *     "version":     10000
 *     "author":      "sqlitestudio.pl"
 * };
 * @endcode
 *
 * and then just declare the class as SQLiteStudio plugin, pointing the json file you just created:
 * @code
 * class MyPlugin : public GenericPlugin, public ScriptingPlugin
 * {
 *     Q_OBJECT
 *     SQLITESTUDIO_PLUGIN("myplugin.json")
 *
 *     // rest of the class
 * };
 * @endcode
 */
class API_EXPORT GenericPlugin : public QObject, public virtual Plugin
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
         * @brief Provides configuration object to use in ConfigDialog.
         * @return This implementation always returns null.
         */
        CfgMain* getMainUiConfig();

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
         * @brief Converts plugin version to human readable format.
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

        /**
         * @brief Loads metadata from given Json object.
         * @param The metadata from json file.
         *
         * This is called by PluginManager.
         */
        void loadMetaData(const QJsonObject& metaData);

    private:
        /**
         * @brief Extracts class meta information with given key.
         * @param key Key to extract.
         * @return Value of the meta information, or null if there's no information with given key.
         *
         * This is a helper method which queries Qt's meta object subsystem for class meta information defined with Q_CLASSINFO.
         */
        const char* getMetaInfo(const QString& key) const;

        QHash<QString,QVariant> metaData;
};

#endif // GENERICPLUGIN_H
