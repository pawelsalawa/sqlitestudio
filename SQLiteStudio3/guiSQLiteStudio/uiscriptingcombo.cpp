#include "uiscriptingcombo.h"
#include "services/pluginmanager.h"
#include "plugins/scriptingplugin.h"
#include <QComboBox>

UiScriptingCombo::UiScriptingCombo()
{
}

const char* UiScriptingCombo::getPropertyName() const
{
    return "ScriptingLangCombo";
}

void UiScriptingCombo::handle(QWidget* widget, const QVariant& value)
{
    QComboBox* cb = dynamic_cast<QComboBox*>(widget);
    if (!cb)
        return;

    if (!value.toBool())
        return;

    for (ScriptingPlugin* plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        cb->addItem(plugin->getLanguage());
}
