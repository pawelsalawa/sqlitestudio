#ifndef POPULATEPLUGIN_H
#define POPULATEPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "plugins/plugin.h"

class CfgMain;
class PopulateEngine;

class API_EXPORT PopulatePlugin : virtual public Plugin
{
    public:
        virtual PopulateEngine* createEngine() = 0;
};

class API_EXPORT PopulateEngine
{
    public:
        virtual ~PopulateEngine() {}

        virtual bool beforePopulating() = 0;
        virtual QVariant nextValue() = 0;
        virtual void afterPopulating() = 0;

        /**
         * @brief Provides config object that holds configuration for populating.
         * @return Config object, or null if the importing with this plugin is not configurable.
         */
        virtual CfgMain* getConfig() = 0;

        /**
         * @brief Provides name of the form to use for configuration of this plugin in the populate dialog.
         * @return Name of the form (toplevel QWidget in the ui file).
         *
         * If populating with this plugin is not configurable (i.e. getConfig() returns null),
         * then this method is not even called, so it can return anything, just to satisfy method
         * return type. In that case good idea is to always return QString::null.
         *
         * @see FormManager
         */
        virtual QString getPopulateConfigFormName() const = 0;

        /**
         * @brief Called when the UI expects any configuration options to be re-validated.
         * @return true if the validation was successful, or false otherwise.
         *
         * When user interacts with the UI in a way that it doesn't change the config values,
         * but it still requires some options to be re-validated, this method is called.
         *
         * It should validate any configuration values defined with CFG_CATEGORY and CFG_ENTRY
         * and post the validation results by calling POPULATE_MANAGER->handleValidationFromPlugin()
         * for every validated CfgEntry.
         *
         * This is also a good idea to connect to the CfgEntry::changed() signal for entries that should be validated
         * and call this method from the slot, so any changes to the configuration values will be
         * immediately validated and reflected on the UI.
         *
         * In this method you can also call POPULATE_MANAGER->configStateUpdateFromPlugin() to adjust options UI
         * to the current config values.
         *
         * Apart from calling POPULATE_MANAGER with validation results, it should also return true or false,
         * according to validation results. The return value is used by the PopulateDialog to tell if the plugin
         * is currently configured correctly, without going into details, without handling signals from POPULATE_MANAGER.
         */
        virtual bool validateOptions() = 0;
};


#endif // POPULATEPLUGIN_H
