#ifndef CUSTOMCONFIGWIDGETPLUGIN_H
#define CUSTOMCONFIGWIDGETPLUGIN_H

#include "plugin.h"
#include <QVariant>

class CfgEntry;
class QWidget;

class CustomConfigWidgetPlugin : public virtual Plugin
{
    public:
        virtual bool isConfigForWidget(CfgEntry* key, QWidget* widget) = 0;
        virtual void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant &value) = 0;
        virtual void saveWidgetToConfig(QWidget* widget, CfgEntry* key) = 0;
        virtual const char* getModifiedNotifier() const = 0;
        virtual QString getFilterString(QWidget* widget) const = 0;
};

#endif // CUSTOMCONFIGWIDGETPLUGIN_H
