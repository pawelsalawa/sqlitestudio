#ifndef PLUGINSERVICEBASE_H
#define PLUGINSERVICEBASE_H

#include <QObject>

class CfgEntry;

class PluginServiceBase : public QObject
{
        Q_OBJECT

    public:
        explicit PluginServiceBase(QObject *parent = 0);

        /**
         * @brief Available for the plugins to report validation errors on their UI forms.
         * @param configValid If the config value is valid or not.
         * @param key The config key that was validated.
         * @param errorMessage if the \p valid is false, then the \p errorMessage can carry the details of the validation result.
         *
         * Since import plugins themself are independet from QtGui, they still can provide *.ui files
         * and they can use CFG_CATEGORIES to bind with *.ui files, then they can validate values
         * stored in the CFG_CATEGORIES. In case that some value is invalid, they should call
         * this method to let the UI know, that the widget should be marked for invalid value.
         */
        void handleValidationFromPlugin(bool configValid, CfgEntry* key, const QString& errorMessage = QString());

        /**
         * @brief Available for the plugins to update UI of their options accordingly to the config values.
         * @param key The config key that the update is about.
         * @param visible The visibility for the widget.
         * @param enabled Enabled/disabled state for the widget.
         *
         * This method is here for the same reason that the handleValidationFromPlugin() is.
         */
        void updateVisibilityAndEnabled(CfgEntry* key, bool visible, bool enabled);

    signals:
        /**
         * @brief Emitted when the plugin performed its configuration validation.
         * @param valid true if plugin accepts its configuration.
         * @param key a key that cause valid/invalid state.
         * @param errorMessage if the \p valid is false, then the \p errorMessage can carry the details of the validation result.
         *
         * Slot handling this signal should update UI to reflect the configuration state.
         */
        void validationResultFromPlugin(bool valid, CfgEntry* key, const QString& errorMessage);

        /**
         * @brief Emitted when the plugin wants to update UI according to config values.
         * @param key The config key that the update is about.
         * @param visible The visibility for the widget.
         * @param enabled Enabled/disabled state for the widget.
         *
         * Slot handling this signal should update UI to reflect the state provided in parameters.
         */
        void stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled);

    public slots:

};

#endif // PLUGINSERVICEBASE_H
