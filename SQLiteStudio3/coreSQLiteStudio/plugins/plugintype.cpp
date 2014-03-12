#include "plugin.h"
#include "plugintype.h"
#include "services/pluginmanager.h"

PluginType::PluginType(const QString& title, const QString& form) :
    title(title), configUiForm(form)
{
}

PluginType::~PluginType()
{
}

QString PluginType::getName() const
{
    return typeName();
}

QString PluginType::getTitle() const
{
    return title;
}

QString PluginType::getConfigUiForm() const
{
    return configUiForm;
}

QList<Plugin*> PluginType::getLoadedPlugins() const
{
    PluginType* type = const_cast<PluginType*>(this);
    return PLUGINS->getLoadedPlugins(type);
}

QStringList PluginType::getAllPluginNames() const
{
    PluginType* type = const_cast<PluginType*>(this);
    return PLUGINS->getAllPluginNames(type);
}
