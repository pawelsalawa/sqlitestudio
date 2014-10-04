#ifndef UICONFIGUREDPLUGIN_H
#define UICONFIGUREDPLUGIN_H

#include "coreSQLiteStudio_global.h"

class API_EXPORT UiConfiguredPlugin
{
    public:
        /**
         * @brief Gets name of the configuration UI form.
         * @return Name of the form object.
         *
         * Some plugins may link (during compilation) only to the coreSQLiteStudio part of the application, but they can still
         * benefit from SQLiteStudio GUI application by providing UI form that will be used in ConfigDialog.
         *
         * This method should return the object name of the top-most widget found in the provided *.ui file.
         *
         * For more details see: http://wiki.sqlitestudio.pl/index.php/Plugin_UI_forms
         */
        virtual QString getConfigUiForm() const = 0;

        /**
         * @brief Provides config object for ConfigDialog.
         * @return Config used by the plugin, or null when there's no config, or when config should not be configured with property binding.
         *
         * When this method returns null, but getConfigUiForm() returns existing form, then configuration is assumed to be kept
         * in global CfgMain object (which is known to application, as all global CfgMain objects). In this case configuration is loaded/stored
         * using initial and final calls to load/store values from the form. This is different from when this method returns not null. Keep reading.
         *
         * When this method returns pointer to an object, then ConfigDialog uses ConfigMapper to bind widgets from getConfigUiForm() with
         * config values from CfgMain returned by this method. See ConfigMapper for details about binding.
         * In this case ConfigDialog uses CfgMain::begin(), CfgMain::commit() and CfgMain::rollback() methods to make changes to the config
         * transactional (when users clicks "cancel" or "apply").
         */
        virtual CfgMain* getMainUiConfig() = 0;

        /**
         * @brief Notifies about ConfigDialog being just open.
         *
         * This is called just after the config dialog was open and all its contents are already initialized.
         * This is a good moment to connect to plugin's CfgMain configuration object to listen for changes,
         * so all uncommited (yet) configuration changes can be reflected by this plugin.
         */
        virtual void configDialogOpen() = 0;

        /**
         * @brief Notifies about ConfigDialog being closed.
         *
         * This is called just before the config dialog gets closed.
         * This is a good moment to disconnect from configuration object and not listen to changes in the configuration anymore
         * (couse config can change for example when application is starting and loading entire configuration, etc).
         */
        virtual void configDialogClosed() = 0;
};

#endif // UICONFIGUREDPLUGIN_H
