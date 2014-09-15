#ifndef LISTTOSTRINGLISTHASH_H
#define LISTTOSTRINGLISTHASH_H

#include "customconfigwidgetplugin.h"
#include "plugins/genericplugin.h"
#include "guiSQLiteStudio_global.h"

class GUI_API_EXPORT ListToStringListHash: public GenericPlugin, public CustomConfigWidgetPlugin
{
    public:
        ListToStringListHash(CfgEntry* key);
        bool isConfigForWidget(CfgEntry* key, QWidget* widget);
        void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value);
        QVariant getWidgetConfigValue(QWidget* widget, bool& ok);
        const char*getModifiedNotifier() const;
        QString getFilterString(QWidget* widget) const;

    private:
        CfgEntry* assignedKey = nullptr;
};

#endif // LISTTOSTRINGLISTHASH_H
