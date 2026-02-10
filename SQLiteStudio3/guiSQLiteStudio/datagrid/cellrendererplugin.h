#ifndef CELLRENDERERPLUGIN_H
#define CELLRENDERERPLUGIN_H

#include "plugins/plugin.h"
#include "datagrid/sqlquerymodelcolumn.h"
#include <QModelIndex>
#include <QStyleOptionViewItem>

class QPainter;
class QAbstractItemDelegate;

/**
 * @brief The CellRendererPlugin class is the interface for plugins that provide custom cell rendering in data grids.
 */
class GUI_API_EXPORT CellRendererPlugin : public virtual Plugin
{
    public:
        /**
         * @brief getPreferredTypes is called by the view to check if this plugin wants to be used for rendering cells of given data type.
         * @return List of data types that this plugin prefers to be used for. If the list is empty,
         * then this plugin does not have a preference and can be used for any data type.
         *
         * This is used to configure default renderer for particular data types, if plugin wants to become one.
         * It is okay to always return empty list and let user configure for which data types this plugin should be used.
         *
         * User still can turn off this renderer for specified data types in the configuration window even if this method returns
         * particular types in the list.
         */
        virtual QList<DataType> getPreferredTypes() = 0;

        /**
         * @brief Creates an instance of QAbstractItemDelegate that will be used for rendering cells.
         * @return Instance of QAbstractItemDelegate for rendering cells.
         *
         * It is created as per-column for each data type that this plugin is configured to be used for.
         * The view does not take ownership over the constructed delegate. This method may return
         * same delegate instance for multiple calls, if the delegate is stateless.
         * It also means that the plugin is responsible for deleting the delegate when it is no longer needed. The plugin should
         * delete the delegate in the deinit() method, which is called when plugin is about to be unloaded.
         *
         * Useful static public methods that may be needed inside the delegate implementation are:
         * SqlQueryItemDelegate::getItem(const QModelIndex &index)
         * SqlQueryItemDelegate::handleUncommitedPainting(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index).
         */
        virtual QAbstractItemDelegate* createDelegate() = 0;

        /**
         * @brief Provides the name of this cell renderer to be shown to user in configuration window.
         * @return Name of this cell renderer.
         */
        virtual QString getRendererName() const = 0;

};
#endif // CELLRENDERERPLUGIN_H
