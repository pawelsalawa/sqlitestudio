#ifndef PLUGINSERVICEBASE_H
#define PLUGINSERVICEBASE_H

#include "common/global.h"
#include <QObject>

class CfgEntry;

class PluginServiceBase : public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Name of property to store scripting language.
         *
         * This property is used by plugins to store scripting language associated with given widget.
         * Upon update of this property, the higlighter can be dynamically changed.
         * Having this in a dynamic property we can keep plugins independent from UI, but they still
         * can interact with the UI.
         */
        static_char* LANG_PROPERTY_NAME = "language";

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
         * @brief Available for the plugins to set custom properties on their UI forms.
         * @param key The config key that the property reffers to (it must be bind to the UI widget).
         * @param propertyName Name of the property to set.
         * @param value Value for the property.
         *
         * This method is here for similar purpose as handleValidationFromPlugin(), just handles different action from the plugin.
         */
        void propertySetFromPlugin(CfgEntry* key, const QString& propertyName, const QVariant& value);

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
         * @brief Emitted when plugin wants to set custom property value for the UI widget.
         * @param key a key that cause valid/invalid state.
         * @param propertyName Name of the property to set.
         * @param value Value for the property.
         *
         * Slot handling this signal should set the property to the widget which is bind to the given key.
         */
        void widgetPropertyFromPlugin(CfgEntry* key, const QString& propertyName, const QVariant& value);

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
