#ifndef MULTIEDITORWIDGETPLUGIN_H
#define MULTIEDITORWIDGETPLUGIN_H

#include "plugins/plugin.h"
#include "datagrid/sqlquerymodelcolumn.h"

class MultiEditorWidget;

/**
 * @brief Plugin interface for MultiEditor editor widgets.
 *
 * Plugins implementing this interface can provide editor widgets
 * to be used in MultiEditor dialog for editing cell values.
 */
class GUI_API_EXPORT MultiEditorWidgetPlugin : public virtual Plugin
{
    public:
        /**
         * @brief Get instance of the editor widget represented by this plugin.
         * @return Instance of the editor widget.
         */
        virtual MultiEditorWidget* getInstance() = 0;

        /**
         * @brief Determine whether this editor is valid for given data type declared for the column.
         * @param dataType Data type of the cell declared on the column.
         * @return True if this editor can be used for given data type, false otherwise.
         */
        virtual bool validFor(const DataType& dataType) = 0;

        /**
         * @brief Determine the priority of this editor for given value and data type.
         * @param value Current value stored in the cell that may be helpful to determin the priority.
         *        Actual value is passed only when testing GridView cell double-click opening.
         *        In regular MultiEditor (on FormView or on explicit MultiEditor opening from GridView) this value will be void.
         * @param dataType Data type of the cell declared on the column that may be helpful to determin the priority.
         * @return Priority value.
         *
         * The lower returned priority value, the higher the priority. Default value for most editors is 10.
         * Higher priority means that tab with this editor will be added earlier to MultiEditor's TabSet.
         * Priority equal to 3 or less means that double-clicking on the cell in Grid View will automatically
         * open MultiEditor with this editor widget, instead of grid's inline editor.
         */
        virtual int getPriority(const QVariant& value, const DataType& dataType) = 0;

        /**
         * @brief Get label for the tab representing this editor in MultiEditor.
         * @return Tab label.
         */
        virtual QString getTabLabel() = 0;
};

#endif // MULTIEDITORWIDGETPLUGIN_H
