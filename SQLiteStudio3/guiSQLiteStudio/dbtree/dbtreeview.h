#ifndef DBTREEVIEW_H
#define DBTREEVIEW_H

#include "dbtree.h"
#include "guiSQLiteStudio_global.h"
#include <QTreeView>
#include <QList>
#include <QUrl>

class QMenu;
class QStandardItemModel;
class DbTreeItemDelegate;

class GUI_API_EXPORT DbTreeView : public QTreeView
{
        Q_OBJECT
    public:
        explicit DbTreeView(QWidget *parent = 0);
        ~DbTreeView();

        void setDbTree(DbTree* dbTree);
        DbTree* getDbTree() const;

        DbTreeItem *currentItem();
        void setCurrentItem(DbTreeItem* item);
        DbTreeItem *currentDbItem();
        DbTreeItem *itemAt(const QPoint& pos);
        QList<DbTreeItem*> selectionItems();
        void selectItems(const QList<DbTreeItem*>& items);
        DbTreeModel *model() const;
        DbTreeItem *getItemForAction(bool onlySelected = false) const;
        QPoint getLastDropPosition() const;
        QModelIndexList getSelectedIndexes() const;

    protected:
        void dragEnterEvent(QDragEnterEvent* e);
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
        void expandToMakeVisible(DbTreeItem* item);

        QMenu* contextMenu = nullptr;
        DbTree* dbTree = nullptr;
        DbTreeItemDelegate* itemDelegate = nullptr;
        QPoint lastDropPosition;

    private slots:
        void showMenu(const QPoint &pos);
        void updateItemHidden(DbTreeItem* item);
};

#endif // DBTREEVIEW_H
