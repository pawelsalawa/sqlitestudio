#ifndef DBTREEVIEW_H
#define DBTREEVIEW_H

#include "dbtree.h"
#include <QTreeView>
#include <QList>
#include <QUrl>

class QMenu;
class QStandardItemModel;
class DbTreeItemDelegate;

class DbTreeView : public QTreeView
{
        Q_OBJECT
    public:
        explicit DbTreeView(QWidget *parent = 0);
        ~DbTreeView();

        void setDbTree(DbTree* dbTree);
        DbTree* getDbTree() const;

        DbTreeItem *currentItem();
        DbTreeItem *itemAt(const QPoint& pos);
        QList<DbTreeItem *> selectionItems();
        DbTreeModel *model() const;
        DbTreeItem *getItemForAction() const;
        QPoint getLastDropPosition() const;
        QModelIndexList getSelectedIndexes() const;

    protected:
        void dragMoveEvent(QDragMoveEvent *event);
        void mouseDoubleClickEvent(QMouseEvent* event);
        void dropEvent(QDropEvent*e);

    private:
        bool handleDoubleClick(DbTreeItem* item);
        bool handleDbDoubleClick(DbTreeItem* item);
        bool handleTableDoubleClick(DbTreeItem* item);
        bool handleIndexDoubleClick(DbTreeItem* item);
        bool handleTriggerDoubleClick(DbTreeItem* item);
        bool handleViewDoubleClick(DbTreeItem* item);
        bool handleColumnDoubleClick(DbTreeItem* item);

        QMenu* contextMenu;
        DbTree* dbTree;
        DbTreeItemDelegate* itemDelegate = nullptr;
        QPoint lastDropPosition;

    private slots:
        void showMenu(const QPoint &pos);
        void updateItemHidden(DbTreeItem* item);
};

#endif // DBTREEVIEW_H
