#include "plugin.h"
#include "plugintype.h"
#include "services/pluginmanager.h"
#include <QDebug>

PluginType::PluginType(const QString& title, const QString& form) :
    title(title), configUiForm(form)
{
}


PluginType::~PluginType()
{
}

QString PluginType::getName() const
{
    return name;
}

void PluginType::setNativeName(const QString& nativeName)
{
    name = nativeName;
    while (name.at(0).isDigit())
        name = name.mid(1);
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

bool PluginType::nameLessThan(PluginType* type1, PluginType* type2)
{
    return type1->title.compare(type2->title) < 0;
}
