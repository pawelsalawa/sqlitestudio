#ifndef CUSTOMCONFIGWIDGETPLUGIN_H
#define CUSTOMCONFIGWIDGETPLUGIN_H

#include "plugins/plugin.h"
#include "guiSQLiteStudio_global.h"
#include <QVariant>

class CfgEntry;
class QWidget;

class GUI_API_EXPORT CustomConfigWidgetPlugin : public virtual Plugin
{
    public:
        virtual bool isConfigForWidget(CfgEntry* key, QWidget* widget) = 0;
        virtual void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant &value) = 0;
        virtual QVariant getWidgetConfigValue(QWidget* widget, bool& ok) = 0;
        virtual const char* getModifiedNotifier() const = 0;
        virtual QString getFilterString(QWidget* widget) const = 0;
};

#endif // CUSTOMCONFIGWIDGETPLUGIN_H
