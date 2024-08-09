#ifndef DBTREE_H
#define DBTREE_H

#include "db/db.h"
#include "common/extactioncontainer.h"
#include "mainwindow.h"
#include "dbtree/dbtreeitem.h"
#include "guiSQLiteStudio_global.h"
#include <QDockWidget>
#include <QSet>
#include <QAtomicInt>

class WidgetCover;
class QAction;
class QMenu;
class DbTreeModel;
class QStandardItem;
class QTimer;
class TableWindow;
class ViewWindow;
class UserInputFilter;
class DbTreeView;


class SqlFileExecutor;
namespace Ui {
    class DbTree;
}

CFG_KEY_LIST(DbTree, QObject::tr("Database list"),
    CFG_KEY_ENTRY(DEL_SELECTED,    Qt::Key_Delete,           QObject::tr("Delete selected item"))
    CFG_KEY_ENTRY(CLEAR_FILTER,    Qt::Key_Escape,           QObject::tr("Clear filter contents"))
    CFG_KEY_ENTRY(REFRESH_SCHEMA,  Qt::Key_F5,               QObject::tr("Refresh schema"))
    CFG_KEY_ENTRY(REFRESH_SCHEMAS, Qt::SHIFT | Qt::Key_F5,   QObject::tr("Refresh all schemas"))
    CFG_KEY_ENTRY(ADD_DB,          Qt::CTRL | Qt::Key_O,     QObject::tr("Add database"))
    CFG_KEY_ENTRY(SELECT_ALL,      Qt::CTRL | Qt::Key_A,     QObject::tr("Select all items"))
    CFG_KEY_ENTRY(COPY,            Qt::CTRL | Qt::Key_C,     QObject::tr("Copy selected item(s)"))
    CFG_KEY_ENTRY(PASTE,           Qt::CTRL | Qt::Key_V,     QObject::tr("Paste from clipboard"))
    CFG_KEY_ENTRY(INCR_FONT_SIZE,  Qt::CTRL | Qt::Key_Plus,  QObject::tr("Increase font size", "database list"))
    CFG_KEY_ENTRY(DECR_FONT_SIZE,  Qt::CTRL | Qt::Key_Minus, QObject::tr("Decrease font size", "database list"))
)

class GUI_API_EXPORT DbTree : public QDockWidget, public ExtActionContainer
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
            ADD_COLUMN,
            EDIT_COLUMN,
            DEL_COLUMN,
            CLEAR_FILTER,
            REFRESH_SCHEMAS,
            REFRESH_SCHEMA,
            CREATE_SIMILAR_TABLE,
            RESET_AUTOINCREMENT,
            ERASE_TABLE_DATA,
            GENERATE_SELECT,
            GENERATE_UPDATE,
            GENERATE_INSERT,
            GENERATE_DELETE,
            OPEN_DB_DIRECTORY,
            EXEC_SQL_FROM_FILE,
            INCR_FONT_SIZE,
            DECR_FONT_SIZE,
            _separator // Never use it directly, it's just for menu setup
        };
        Q_ENUM(Action)

        enum ToolBar
        {
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
        DbTreeView* getView() const;
        void showRefreshWidgetCover();
        void hideRefreshWidgetCover();
        void setSelectedItem(DbTreeItem* item);
        bool isMimeDataValidForItem(const QMimeData* mimeData, const DbTreeItem* item, bool forPasting = false);
        QToolBar* getToolBar(int toolbar) const;
        Db* getSelectedDb();
        Db* getSelectedOpenDb();
        QSet<Db*> getSelectedDatabases();

        static bool isItemDraggable(const DbTreeItem* item);

    protected:
        void createActions();
        void setupDefShortcuts();

    private:
        typedef std::function<bool(DbTreeItem*)> ItemFilterFunc;

        void setActionEnabled(int action, bool enabled);
        TableWindow* openTable(DbTreeItem* item);
        TableWindow* openTable(Db* db, const QString& database, const QString& table);
        TableWindow* newTable(Db* db);
        ViewWindow* openView(DbTreeItem* item);
        ViewWindow* openView(Db* db, const QString& database, const QString& view);
        ViewWindow* newView(Db* db);
        void editIndex(DbTreeItem* item);
        void editTrigger(DbTreeItem* item);
//        void delSelectedObject();
        void filterUndeletableItems(QList<DbTreeItem*>& items);
        void filterItemsWithParentInList(QList<DbTreeItem*>& items);
        void deleteItem(DbTreeItem* item);
        void deleteSelected(DbTreeItem::Type itemType);
        QHash<Db*, QList<DbTreeItem*>> groupByDb(const QList<DbTreeItem*> items);
        QStringList itemsToNames(const QList<DbTreeItem*>& items);
        QString getSelectedTableName() const;
        QString getSelectedIndexName() const;
        QString getSelectedTriggerName() const;
        QString getSelectedViewName() const;
        QList<DbTreeItem*> getSelectedItems(DbTreeItem::Type itemType);
        QList<DbTreeItem*> getSelectedItems(ItemFilterFunc filterFunc = nullptr);
        void changeFontSize(int factor);

        static bool areDbTreeItemsValidForItem(QList<DbTreeItem*> srcItems, const DbTreeItem* dstItem, bool forPasting = false);
        static bool areUrlsValidForItem(const QList<QUrl>& srcUrls, const DbTreeItem* dstItem);
        static void initDndTypes();

        Ui::DbTree *ui = nullptr;
        DbTreeModel* treeModel = nullptr;
        WidgetCover* treeRefreshWidgetCover = nullptr;
        WidgetCover* fileExecWidgetCover = nullptr;
        SqlFileExecutor* fileExecutor = nullptr;

        static QHash<DbTreeItem::Type,QList<DbTreeItem::Type>> allowedTypesInside;
        static QSet<DbTreeItem::Type> draggableTypes;
        static const constexpr int ITEM_TEXT_LIMIT = 300;

    public slots:
        void refreshSchema(Db* db);
        void refreshSchemas();
        void interrupt();
        void updateActionsForCurrent();

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
        void addColumn();
        void editColumn();
        void delColumn();
        void vacuumDb();
        void integrityCheck();
        void createSimilarTable();
        void resetAutoincrement();
        void eraseTableData();
        void addColumn(DbTreeItem* item);
        void editColumn(DbTreeItem* item);
        void delColumn(DbTreeItem* item);
        void currentChanged(const QModelIndex & current, const QModelIndex & previous);
        void deleteSelected(ItemFilterFunc filterFunc = nullptr);
        void deleteItems(const QList<DbTreeItem*>& itemsToDelete);
        void refreshSchema();
        void dbConnected(Db* db);
        void dbDisconnected(Db* db);
        void updateDbIcon(Db* db);
        void refreshFont();
        void generateSelectForTable();
        void generateInsertForTable();
        void generateUpdateForTable();
        void generateDeleteForTable();
        void openDbDirectory();
        void execSqlFromFile();
        void setFileExecProgress(int newValue);
        void hideFileExecCover();
        void showFileExecErrors(const QList<QPair<QString, QString>>& errors, bool rolledBack);
        void fontSizeChangeRequested(int delta);
        void incrFontSize();
        void decrFontSize();

    signals:
        void updateFileExecProgress(int value);
        void fileExecCoverToBeClosed();
        void fileExecErrors(const QList<QPair<QString, QString>>& errors, bool rolledBack);
        void sessionValueChanged();
};

TYPE_OF_QHASH qHash(DbTree::Action action);

#define DBTREE MainWindow::getInstance()->getDbTree()

#endif // DBTREE_H
