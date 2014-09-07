#ifndef MULTIEDITORWIDGETPLUGIN_H
#define MULTIEDITORWIDGETPLUGIN_H

#include "plugins/plugin.h"
#include "datagrid/sqlquerymodelcolumn.h"

class MultiEditorWidget;

class GUI_API_EXPORT MultiEditorWidgetPlugin : public virtual Plugin
{
    public:
        virtual MultiEditorWidget* getInstance() = 0;
        virtual bool validFor(const DataType& dataType) = 0;
        virtual int getPriority(const DataType& dataType) = 0;
};

#endif // MULTIEDITORWIDGETPLUGIN_H
