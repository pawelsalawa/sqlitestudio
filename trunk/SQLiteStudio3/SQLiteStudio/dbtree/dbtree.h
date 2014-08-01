#ifndef DBTREE_H
#define DBTREE_H

#include "db/db.h"
#include "common/extactioncontainer.h"
#include "mainwindow.h"
#include "dbtree/dbtreeitem.h"
#include <QDockWidget>

class WidgetCover;
class QAction;
class QMenu;
class DbTreeModel;
class QStandardItem;
class QTimer;
class TableWindow;
class ViewWindow;
class UserInputFilter;

namespace Ui {
    class DbTree;
}

CFG_KEY_LIST(DbTree, QObject::tr("Database list"),
    CFG_KEY_ENTRY(DEL_SELECTED,    Qt::Key_Delete,         QObject::tr("Delete selected item"))
    CFG_KEY_ENTRY(CLEAR_FILTER,    Qt::Key_Escape,         QObject::tr("Clear filter contents"))
    CFG_KEY_ENTRY(REFRESH_SCHEMA,  Qt::Key_F5,             QObject::tr("Refresh schema"))
    CFG_KEY_ENTRY(REFRESH_SCHEMAS, Qt::SHIFT + Qt::Key_F5, QObject::tr("Refresh all schemas"))
    CFG_KEY_ENTRY(ADD_DB,          Qt::CTRL + Qt::Key_O,   QObject::tr("Add database"))
    CFG_KEY_ENTRY(SELECT_ALL,      Qt::CTRL + Qt::Key_A,   QObject::tr("Select all items"))
    CFG_KEY_ENTRY(COPY,            Qt::CTRL + Qt::Key_C,   QObject::tr("Copy selected item(s)"))
    CFG_KEY_ENTRY(PASTE,           Qt::CTRL + Qt::Key_V,   QObject::tr("Paste from clipboard"))
)

class DbTree : public QDockWidget, public ExtActionContainer
{
        Q_OBJECT
        Q_ENUMS(Action)

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
            CONVERT_DB,
            VACUUM_DB,
            INTEGRITY_CHECK,
            ADD_TABLE,
            EDIT_TABLE,
            DEL_TABLE,
            EXPORT_TABLE,
            IMPORT_TABLE,
            POPULATE_TABLE,
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

        static void staticInit();

        void init();
        void updateActionStates(const QStandardItem* item);
        void setupActionsForMenu(DbTreeItem* currItem, QMenu* contextMenu);
        QVariant saveSession();
        void restoreSession(const QVariant& sessionValue);
        DbTreeModel* getModel() const;
        void showWidgetCover();
        void hideWidgetCover();
        void setSelectedItem(DbTreeItem* item);

        static bool isMimeDataValidForItem(const QMimeData* mimeData, const DbTreeItem* item);
        static bool isItemDraggable(const DbTreeItem* item);

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
        static bool areDbTreeItemsValidForItem(QList<DbTreeItem*> srcItems, const DbTreeItem* dstItem);
        static bool areUrlsValidForItem(const QList<QUrl>& srcUrls, const DbTreeItem* dstItem);

        static void initDndTypes();

        Ui::DbTree *ui;
        DbTreeModel* treeModel;
        WidgetCover* widgetCover = nullptr;

        static QHash<DbTreeItem::Type,QList<DbTreeItem::Type>> allowedTypesInside;
        static QSet<DbTreeItem::Type> draggableTypes;

    public slots:
        void refreshSchema(Db* db);
        void refreshSchemas();
        void interrupt();

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
        void populateTable();
        void editColumn();
        void convertDb();
        void vacuumDb();
        void integrityCheck();
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
