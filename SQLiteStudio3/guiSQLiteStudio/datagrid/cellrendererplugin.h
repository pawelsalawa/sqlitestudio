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
         * @brief useFor Determines whether this plugin should be used for rendering cells of given data type.
         * @param dataType Data type of the cell to be rendered.
         * @return True if this plugin should be used for rendering cells of given data type, false otherwise.
         *
         * This is used to configure default renderer for particular data types, if plugin wants to become one.
         * It is okay to always return false and let user configure for which data types this plugin should be used.
         *
         * User still can turn off this renderer for specified data types in the configuration window even if this method returns true.
         */
        virtual bool useFor(const DataType& dataType) = 0;

        /**
         * @brief Creates an instance of QAbstractItemDelegate that will be used for rendering cells.
         * @return Instance of QAbstractItemDelegate for rendering cells.
         *
         * It is created as per-column for each data type that this plugin is configured to be used for.
         * The view does not take ownership over the constructed delegate. This method may return
         * same delegate instance for multiple calls, if the delegate is stateless.
         *
         * Useful static public methods that may be needed inside the delegate implementation are:
         * SqlQueryItemDelegate::getItem(const QModelIndex &index)
         * SqlQueryItemDelegate::handleUncommitedPainting(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index).
         */
        virtual QAbstractItemDelegate* createDelegate() = 0;

        /**
         * @brief Should delete all delegates it has created in createDelegate().
         *
         * This is called just before the plugin is unloaded to allow it to cleanup any resources.
         * At this point all views have stopped using this delegate, so it's safe to delete them.
         */
        virtual void cleanup() = 0;

        /**
         * @brief Provides the name of this cell renderer to be shown to user in configuration window.
         * @return Name of this cell renderer.
         */
        virtual QString getRendererName() const = 0;

};
#endif // CELLRENDERERPLUGIN_H
