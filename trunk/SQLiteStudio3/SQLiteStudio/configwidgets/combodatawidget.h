#ifndef COMBODATAWIDGET_H
#define COMBODATAWIDGET_H

#include "customconfigwidgetplugin.h"
#include "plugins/genericplugin.h"

class ComboDataWidget : public GenericPlugin, public CustomConfigWidgetPlugin
{
    public:
        explicit ComboDataWidget(CfgEntry* key);

        bool isConfigForWidget(CfgEntry* key, QWidget* widget);
        void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value);
        void saveWidgetToConfig(QWidget* widget, CfgEntry* key);
        const char*getModifiedNotifier() const;
        QString getFilterString(QWidget* widget) const;

    private:
        CfgEntry* assignedKey = nullptr;
};

#endif // COMBODATAWIDGET_H
