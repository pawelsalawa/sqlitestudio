#ifndef DBTREE_H
#define DBTREE_H

#include "db/db.h"
#include "common/extactioncontainer.h"
#include "mainwindow.h"
#include <QDockWidget>

class QAction;
class QMenu;
class DbTreeModel;
class DbTreeItem;
class QStandardItem;
class QTimer;
class TableWindow;
class ViewWindow;
class UserInputFilter;

namespace Ui {
    class DbTree;
}

class DbTree : public QDockWidget, public ExtActionContainer
{
        Q_OBJECT

    public:
        friend class DbTreeView;

        enum Action
        {
            COPY,
            PASTE,
            SELECT_ALL,
            DEL_SELECTED,
            CREATE_GROUP,
            DELETE_GROUP,
            RENAME_GROUP,
            ADD_DB,
            EDIT_DB,
            DELETE_DB,
            CONNECT_TO_DB,
            DISCONNECT_FROM_DB,
            IMPORT_INTO_DB,
            EXPORT_DB,
            ADD_TABLE,
            EDIT_TABLE,
            DEL_TABLE,
            EXPORT_TABLE,
            IMPORT_TABLE,
            ADD_INDEX,
            EDIT_INDEX,
            DEL_INDEX,
            ADD_TRIGGER,
            EDIT_TRIGGER,
            DEL_TRIGGER,
            ADD_VIEW,
            EDIT_VIEW,
            DEL_VIEW,
            EDIT_COLUMN,
            CLEAR_FILTER,
            REFRESH_SCHEMAS,
            REFRESH_SCHEMA,
            _separator // Never use it directly, it's just for menu setup
        };

        explicit DbTree(QWidget *parent = 0);
        ~DbTree();
        void init();
        void updateActionStates(const QStandardItem* item);
        void setupActionsForMenu(DbTreeItem* currItem, QMenu* contextMenu);
        QVariant saveSession();
        void restoreSession(const QVariant& sessionValue);
        DbTreeModel* getModel() const;

    protected:
        void createActions();
        void setupDefShortcuts();

    private:
        void setActionEnabled(int action, bool enabled);
        Db* getSelectedDb();
        Db* getSelectedOpenDb();
        TableWindow* openTable(DbTreeItem* item);
        TableWindow* openTable(Db* db, const QString& database, const QString& table);
        TableWindow* newTable(DbTreeItem* item);
        ViewWindow* openView(DbTreeItem* item);
        ViewWindow* openView(Db* db, const QString& database, const QString& view);
        ViewWindow* newView(DbTreeItem* item);
        void editIndex(DbTreeItem* item);
        void editTrigger(DbTreeItem* item);
        void delSelectedObject();
        void filterUndeletableItems(QList<DbTreeItem*>& items);
        void filterItemsWithParentInList(QList<DbTreeItem*>& items);
        void deleteItem(DbTreeItem* item);

        Ui::DbTree *ui;
        DbTreeModel* treeModel;

    public slots:
        void refreshSchema(Db* db);
        void refreshSchemas();

    private slots:
        void copy();
        void paste();
        void selectAll();
        void createGroup();
        void deleteGroup();
        void renameGroup();
        void addDb();
        void editDb();
        void removeDb();
        void connectToDb();
        void disconnectFromDb();
        void import();
        void exportDb();
        void addTable();
        void editTable();
        void delTable();
        void addIndex();
        void editIndex();
        void delIndex();
        void addTrigger();
        void editTrigger();
        void delTrigger();
        void addView();
        void editView();
        void delView();
        void exportTable();
        void importTable();
        void editColumn();
        void editColumn(DbTreeItem* item);
        void currentChanged(const QModelIndex & current, const QModelIndex & previous);
        void deleteSelected();
        void deleteItems(const QList<DbTreeItem*>& itemsToDelete);
        void refreshSchema();
        void updateActionsForCurrent();
};

int qHash(DbTree::Action action);

#define DBTREE MainWindow::getInstance()->getDbTree()

#endif // DBTREE_H
