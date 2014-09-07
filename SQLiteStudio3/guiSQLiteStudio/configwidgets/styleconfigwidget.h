#ifndef STYLECONFIGWIDGET_H
#define STYLECONFIGWIDGET_H

#include "plugins/genericplugin.h"
#include "customconfigwidgetplugin.h"
#include "guiSQLiteStudio_global.h"

class GUI_API_EXPORT StyleConfigWidget : public GenericPlugin, public CustomConfigWidgetPlugin
{
    public:
        StyleConfigWidget();

        bool isConfigForWidget(CfgEntry* key, QWidget* widget);
        void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value);
        void saveWidgetToConfig(QWidget* widget, CfgEntry* key);
        const char* getModifiedNotifier() const;
        QString getFilterString(QWidget* widget) const;
};

#endif // STYLECONFIGWIDGET_H
