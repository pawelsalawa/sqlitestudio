#ifndef COMBODATAWIDGET_H
#define COMBODATAWIDGET_H

#include "customconfigwidgetplugin.h"
#include "plugins/genericplugin.h"
#include "guiSQLiteStudio_global.h"

/**
 * @brief Config entry handler for combo box items with dynamic data set
 *
 * This config entry handler runs only for specified "assigned key", so even it's implements CustomConfigWidgetPlugin,
 * it's created explicitly for each combo.
 *
 * It is used to convert CfgEntry value to one of combo's entries and set that value in the combo.
 * It also works the other way, of course (from combo value to CfgEntry value).
 *
 * Currently it is used only by ConfigDialog because of its specific case with custom formatter combo,
 * which has dynamic contents based on what's added/removed from the combo.
 */
class GUI_API_EXPORT ComboDataWidget : public GenericPlugin, public CustomConfigWidgetPlugin
{
    public:
        explicit ComboDataWidget(CfgEntry* key);

        bool isConfigForWidget(CfgEntry* key, QWidget* widget);
        void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value);
        QVariant getWidgetConfigValue(QWidget* widget, bool& ok);
        const char*getModifiedNotifier() const;
        QString getFilterString(QWidget* widget) const;

    private:
        CfgEntry* assignedKey = nullptr;
};

#endif // COMBODATAWIDGET_H
