#ifndef CONFIGUIPLUGIN_H
#define CONFIGUIPLUGIN_H

#include <QtPlugin>

class CfgEntry;

class ConfigUiPlugin
{
    public:
        virtual ~ConfigUiPlugin() {}

        virtual bool isEligible(CfgEntry* key, QWidget *widget, const QVariant& value) const = 0;
        virtual void loadConfigToWidget(CfgEntry* key, QWidget *widget, const QVariant& value) = 0;
        virtual QVariant saveConfigFromWidget(CfgEntry* key, QWidget *widget) = 0;
        virtual bool init() = 0;
        virtual void deinit() = 0;
};

#define ConfigUiPluginInterface "pl.sqlitestudio.ConfigUiPlugin/1.0"
Q_DECLARE_INTERFACE(ConfigUiPlugin, ConfigUiPluginInterface)


#endif // CONFIGUIPLUGIN_H
