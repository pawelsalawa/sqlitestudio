#ifndef MULTIEDITORWIDGETPLUGIN_H
#define MULTIEDITORWIDGETPLUGIN_H

#include "multieditorwidget.h"

class MultiEditorWidgetPlugin
{
    public:
        virtual ~MultiEditorWidgetPlugin() {}

        virtual MultiEditorWidget* getInstance() = 0;
        virtual bool validFor(const QVariant& value) = 0;
        virtual QString getName() = 0;
};

#define MultiEditorWidgetInterface "pl.sqlitestudio.MultiEditorWidgetPlugin/1.0"
Q_DECLARE_INTERFACE(MultiEditorWidgetPlugin, MultiEditorWidgetInterface)

#endif // MULTIEDITORWIDGETPLUGIN_H
