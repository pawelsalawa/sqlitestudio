#include "dbtree.h"
#include "dbtreeitem.h"
#include "ui_dbtree.h"
#include "utils_sql.h"
#include "sqlitestudio.h"
#include "dbtreemodel.h"
#include "dialogs/dbdialog.h"
#include "db/dbmanager.h"
#include "iconmanager.h"
#include "global.h"
#include "notifymanager.h"
#include "mainwindow.h"
#include "mdiarea.h"
#include "unused.h"
#include "dbobjectdialogs.h"
#include "common/userinputfilter.h"
#include "windows/tablewindow.h"
#include "dialogs/indexdialog.h"
#include "dialogs/triggerdialog.h"
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

DbTree::DbTree(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DbTree)
{
    init();
}

DbTree::~DbTree()
{
    delete ui;
    delete treeModel;
}

void DbTree::init()
{
    ui->setupUi(this);

    ui->nameFilter->setClearButtonEnabled(true);

    treeModel = new DbTreeModel();
    treeModel->setTreeView(ui->treeView);

    new UserInputFilter(ui->nameFilter, treeModel, SLOT(applyFilter(QString)));

    ui->treeView->setDbTree(this);
    ui->treeView->setModel(treeModel);

    initActions();

    if (DBLIST->getDbList().size() > 0)
        treeModel->loadDbList();

    connect(DBLIST, SIGNAL(dbListLoaded()), treeModel, SLOT(loadDbList()));

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DbTree::currentChanged);
    connect(DBLIST, SIGNAL(dbConnected(Db*)), this, SLOT(updateActionsForCurrent()));
    connect(DBLIST, SIGNAL(dbDisconnected(Db*)), this, SLOT(updateActionsForCurrent()));

    updateActionsForCurrent();
}

void DbTree::createActions()
{
    createAction(COPY, "act_copy", tr("Copy"), this, SLOT(copy()), this);
    createAction(PASTE, "act_paste", tr("Paste"), this, SLOT(paste()), this);
    createAction(SELECT_ALL, "act_select_all", tr("Select all"), this, SLOT(selectAll()), this);
    createAction(CREATE_GROUP, "directory_add", tr("Create a group"), this, SLOT(createGroup()), this);
    createAction(DELETE_GROUP, "directory_del", tr("Delete the group"), this, SLOT(deleteGroup()), this);
    createAction(RENAME_GROUP, "directory_edit", tr("Rename the group"), this, SLOT(renameGroup()), this);
    createAction(ADD_DB, "database_add", tr("Add a database"), this, SLOT(addDb()), this);
    createAction(EDIT_DB, "database_edit", tr("Edit the database"), this, SLOT(editDb()), this);
    createAction(DELETE_DB, "database_del", tr("Remove the database"), this, SLOT(removeDb()), this);
    createAction(CONNECT_TO_DB, "database_connect", tr("Connect to the database"), this, SLOT(connectToDb()), this);
    createAction(DISCONNECT_FROM_DB, "database_disconnect", tr("Disconnect from the database"), this, SLOT(disconnectFromDb()), this);
    createAction(ADD_TABLE, "table_add", tr("Create a table"), this, SLOT(addTable()), this);
    createAction(EDIT_TABLE, "table_edit", tr("Edit the table"), this, SLOT(editTable()), this);
    createAction(DEL_TABLE, "table_del", tr("Drop the table"), this, SLOT(delTable()), this);
    createAction(ADD_INDEX, "index_add", tr("Create an index"), this, SLOT(addIndex()), this);
    createAction(EDIT_INDEX, "index_edit", tr("Edit the index"), this, SLOT(editIndex()), this);
    createAction(DEL_INDEX, "index_del", tr("Drop the index"), this, SLOT(delIndex()), this);
    createAction(ADD_TRIGGER, "trigger_add", tr("Create a trigger"), this, SLOT(addTrigger()), this);
    createAction(EDIT_TRIGGER, "trigger_edit", tr("Edit the trigger"), this, SLOT(editTrigger()), this);
    createAction(DEL_TRIGGER, "trigger_del", tr("Drop the trigger"), this, SLOT(delTrigger()), this);
    createAction(ADD_VIEW, "view_add", tr("Create a view"), this, SLOT(addView()), this);
    createAction(EDIT_VIEW, "view_edit", tr("Edit the view"), this, SLOT(editView()), this);
    createAction(DEL_VIEW, "view_del", tr("Drop the view"), this, SLOT(delView()), this);
    createAction(EDIT_COLUMN, "column_edit", tr("Edit the column"), this, SLOT(editColumn()), this);
    createAction(DEL_SELECTED, "act_select_all", tr("Select all"), this, SLOT(deleteSelected()), this);
    createAction(CLEAR_FILTER, tr("Clear filter"), ui->nameFilter, SLOT(clear()), this);
    createAction(REFRESH_SCHEMAS, "database_reload", tr("Refresh all database schemas"), treeModel, SLOT(loadDbList()), this);
    createAction(REFRESH_SCHEMA, "database_reload", tr("Refresh database schema"), this, SLOT(refreshSchema()), this);
}

void DbTree::updateActionsFor(const QStandardItem *item)
{
    // TODO update statuses of "List" submenu (copy, paste)
    QList<int> enabled;
    const DbTreeItem* dbTreeItem = dynamic_cast<const DbTreeItem*>(item);
    if (item != nullptr)
    {
        bool isDbOpen = false;
        DbTreeItem* parentItem = dbTreeItem->parentDbTreeItem();
        DbTreeItem* grandParentItem = parentItem ? parentItem->parentDbTreeItem() : nullptr;

        // Add database should always be available
        enabled << ADD_DB;

        // Deleting any item should be enabled if just any is selected.
        enabled << DEL_SELECTED << CLEAR_FILTER;

        // Group actions
        if (dbTreeItem->getType() == DbTreeItem::Type::DIR)
            enabled << CREATE_GROUP << RENAME_GROUP << DELETE_GROUP << ADD_DB;

        if (dbTreeItem->getDb())
        {
            enabled << DELETE_DB << EDIT_DB;
            if (dbTreeItem->getDb()->isOpen())
            {
                enabled << DISCONNECT_FROM_DB << ADD_TABLE << ADD_VIEW;
                enabled << REFRESH_SCHEMA;
                isDbOpen = true;
            }
            else
                enabled << CONNECT_TO_DB;
        }

        if (isDbOpen)
        {
            switch (dbTreeItem->getType())
            {
                case DbTreeItem::Type::ITEM_PROTOTYPE:
                    break;
                case DbTreeItem::Type::DIR:
                    // It's handled outside of "item with db", above
                    break;
                case DbTreeItem::Type::DB:
                    enabled << CREATE_GROUP;
                    break;
                case DbTreeItem::Type::TABLES:
                    break;
                case DbTreeItem::Type::TABLE:
                    enabled << EDIT_TABLE << DEL_TABLE;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
                case DbTreeItem::Type::INDEXES:
                    enabled << EDIT_TABLE << DEL_TABLE;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
                case DbTreeItem::Type::INDEX:
                    enabled << EDIT_TABLE << DEL_TABLE;
                    enabled << EDIT_INDEX << DEL_INDEX;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
                case DbTreeItem::Type::TRIGGERS:
                {
                    if (parentItem->getType() == DbTreeItem::Type::TABLE)
                    {
                        enabled << EDIT_TABLE << DEL_TABLE;
                        enabled << ADD_INDEX << ADD_TRIGGER;
                    }
                    else
                    {
                        enabled << EDIT_VIEW << DEL_VIEW;
                        enabled << ADD_TRIGGER;
                    }

                    enabled << ADD_TRIGGER;
                    break;
                }
                case DbTreeItem::Type::TRIGGER:
                {
                    if (grandParentItem->getType() == DbTreeItem::Type::TABLE)
                    {
                        enabled << EDIT_TABLE << DEL_TABLE;
                        enabled << ADD_INDEX << ADD_TRIGGER;
                    }
                    else
                    {
                        enabled << EDIT_VIEW << DEL_VIEW;
                        enabled << ADD_TRIGGER;
                    }

                    enabled << EDIT_TRIGGER << DEL_TRIGGER;
                    break;
                }
                case DbTreeItem::Type::VIEWS:
                    break;
                case DbTreeItem::Type::VIEW:
                    enabled << EDIT_VIEW << DEL_VIEW;
                    enabled << ADD_TRIGGER;
                    break;
                case DbTreeItem::Type::COLUMNS:
                    enabled << EDIT_TABLE << DEL_TABLE;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
                case DbTreeItem::Type::COLUMN:
                    enabled << EDIT_TABLE << DEL_TABLE;
                    enabled << EDIT_COLUMN;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        enabled << CREATE_GROUP << ADD_DB;
    }

    if (treeModel->rowCount() > 0)
        enabled << SELECT_ALL; // if there's at least 1 item, enable this

    enabled << REFRESH_SCHEMAS;

    foreach (int action, actionMap.keys())
        setActionEnabled(action, enabled.contains(action));
}

QVariant DbTree::saveSession()
{
    treeModel->storeGroups();
    return QVariant();
}

void DbTree::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
}

DbTreeModel* DbTree::getModel() const
{
    return treeModel;
}

void DbTree::setActionEnabled(int action, bool enabled)
{
    actionMap[action]->setEnabled(enabled);
}

Db* DbTree::getSelectedDb()
{
    DbTreeItem* item = ui->treeView->currentItem();
    if (!item)
        return nullptr;

    return item->getDb();
}

Db* DbTree::getSelectedOpenDb()
{
    Db* db = getSelectedDb();
    if (!db || !db->isOpen())
        return nullptr;

    return db;
}

TableWindow* DbTree::openTable(DbTreeItem* item)
{
    QString database = QString::null; // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();
    return openTable(db, database, item->text());
}

TableWindow* DbTree::openTable(Db* db, const QString& database, const QString& table)
{
    DbObjectDialogs dialogs(db);
    return dialogs.editTable(database, table);
}

void DbTree::editIndex(DbTreeItem* item)
{
    //QString database = QString::null; // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();

    DbObjectDialogs dialogs(db);
    dialogs.editIndex(item->text());
}

ViewWindow* DbTree::openView(DbTreeItem* item)
{
    QString database = QString::null; // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();
    return openView(db, database, item->text());
}

ViewWindow* DbTree::openView(Db* db, const QString& database, const QString& view)
{
    DbObjectDialogs dialogs(db);
    return dialogs.editView(database, view);
}

TableWindow* DbTree::newTable(DbTreeItem* item)
{
    Db* db = item->getDb();

    DbObjectDialogs dialogs(db);
    return dialogs.addTable();
}

ViewWindow* DbTree::newView(DbTreeItem* item)
{
    Db* db = item->getDb();

    DbObjectDialogs dialogs(db);
    return dialogs.addView();
}

void DbTree::editTrigger(DbTreeItem* item)
{
    //QString database = QString::null; // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();

    DbObjectDialogs dialogs(db);
    dialogs.editTrigger(item->text());
}

void DbTree::delSelectedObject()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    if (!item)
        return;

    DbObjectDialogs dialogs(db);
    dialogs.dropObject(item->text()); // TODO add database prefix when supported
}

void DbTree::filterUndeletableItems(QList<DbTreeItem*>& items)
{
    QMutableListIterator<DbTreeItem*> it(items);
    DbTreeItem::Type type;
    while (it.hasNext())
    {
        type = it.next()->getType();
        switch (type)
        {
            case DbTreeItem::Type::TABLES:
            case DbTreeItem::Type::INDEXES:
            case DbTreeItem::Type::TRIGGERS:
            case DbTreeItem::Type::VIEWS:
            case DbTreeItem::Type::COLUMNS:
            case DbTreeItem::Type::ITEM_PROTOTYPE:
                it.remove();
                break;
            case DbTreeItem::Type::DIR:
            case DbTreeItem::Type::DB:
            case DbTreeItem::Type::INVALID_DB:
            case DbTreeItem::Type::TABLE:
            case DbTreeItem::Type::INDEX:
            case DbTreeItem::Type::TRIGGER:
            case DbTreeItem::Type::VIEW:
            case DbTreeItem::Type::COLUMN:
                break;
        }
    }
}

void DbTree::filterItemsWithParentInList(QList<DbTreeItem*>& items)
{
    QMutableListIterator<DbTreeItem*> it(items);
    DbTreeItem* item;
    DbTreeItem* pathItem;
    while (it.hasNext())
    {
        item = it.next();
        foreach (pathItem, item->getPathToRoot().mid(1))
        {
            if (items.contains(pathItem) && pathItem->getType() != DbTreeItem::Type::DIR)
            {
                it.remove();
                break;
            }
        }
    }
}

void DbTree::deleteItem(DbTreeItem* item)
{
    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
            treeModel->deleteGroup(item);
            break;
        case DbTreeItem::Type::DB:
            DBLIST->removeDb(item->getDb());
            break;
        case DbTreeItem::Type::INVALID_DB:
            CFG->removeDb(item->text());
            break;
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::INDEX:
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEW:
        {
            Db* db = item->getDb();
            DbObjectDialogs dialogs(db);
            dialogs.dropObject(item->text()); // TODO database name when supported
            break;
        }
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::VIEWS:
        case DbTreeItem::Type::COLUMNS:
        case DbTreeItem::Type::COLUMN:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }
}

void DbTree::refreshSchema(Db* db)
{
    if (!db)
        return;

    if (!db->isOpen())
        return;

    treeModel->refreshSchema(db);
}

void DbTree::copy()
{
    // TODO implement DbTree::copy()
}

void DbTree::paste()
{
    // TODO implement DbTree::paste()
}

void DbTree::selectAll()
{
    ui->treeView->selectAll();
}

void DbTree::createGroup()
{
    QStringList existingItems;
    QStandardItem* currItem = ui->treeView->getItemForAction();
    DbTreeItem* itemToMove = nullptr;
    if (currItem)
    {
        // Look for any directory in the path to the root, starting with the current item
        do
        {
            if (dynamic_cast<DbTreeItem*>(currItem)->getType() == DbTreeItem::Type::DIR)
            {
                existingItems = dynamic_cast<DbTreeItem*>(currItem)->childNames();
                break;
            }
            else
            {
                itemToMove = dynamic_cast<DbTreeItem*>(currItem);
            }
        }
        while ((currItem = currItem->parent()) != nullptr);
    }

    // No luck? Use root.
    if (!currItem)
        currItem = treeModel->root();

    QString name = "";
    while (existingItems.contains(name = QInputDialog::getText(this, tr("Create group"), tr("Group name"))) ||
           (name.isEmpty() && !name.isNull()))
    {
        QMessageBox::information(this, tr("Create directory"), tr("Entry with name %1 already exists in directory %2.")
                                 .arg(name).arg(currItem->text()), QMessageBox::Ok);
    }

    if (name.isNull())
        return;

    DbTreeItem* newDir = treeModel->createGroup(name, currItem);
    if (itemToMove)
        treeModel->move(itemToMove, newDir);
}

void DbTree::deleteGroup()
{
    DbTreeItem* item = ui->treeView->getItemForAction();
    if (!item)
        return;

    QMessageBox::StandardButton resp = QMessageBox::question(this, tr("Delete group"),
            tr("Are you sure you want to delete group %1?\nAll objects from this group will be moved to parent group.").arg(item->text()));

    if (resp != QMessageBox::Yes)
        return;

    treeModel->deleteGroup(item);
}

void DbTree::renameGroup()
{
    DbTreeItem* item = ui->treeView->getItemForAction();
    if (!item)
        return;

    ui->treeView->edit(item->index());
}

void DbTree::addDb()
{
    DbTreeItem* currItem = ui->treeView->getItemForAction();

    DbDialog dialog(DbDialog::ADD, this);
    if (!dialog.exec())
        return;

    QString name = dialog.getName();

    // If we created db in some group, move it there
    if (currItem && currItem->getType() == DbTreeItem::Type::DIR)
    {
        DbTreeItem* dbItem = dynamic_cast<DbTreeItem*>(treeModel->findItem(DbTreeItem::Type::DB, name));
        if (!dbItem)
        {
            qWarning() << "Created and added db to tree, but could not find it while trying to move it to target group" << currItem->text();
            return;
        }
        treeModel->move(dbItem, currItem);
    }
}

void DbTree::editDb()
{
    Db* db = getSelectedDb();
    if (!db)
        return;

    bool perm = SQLiteStudio::getInstance()->getConfig()->isDbInConfig(db->getName());

    DbDialog dialog(DbDialog::EDIT, this);
    dialog.setDb(db);
    dialog.setPermanent(perm);
    dialog.exec();
}

void DbTree::removeDb()
{
    Db* db = getSelectedDb();
    if (!db)
        return;

    int res = QMessageBox::question(this, tr("Delete database"), tr("Are you sure you want to delete database '%1'?").arg(db->getName()));
    if (!res)
        return;

    DBLIST->removeDb(db);
}

void DbTree::connectToDb()
{
    Db* db = getSelectedDb();
    if (!db)
        return;

    if (db->isOpen())
        return;

    db->open();
}

void DbTree::disconnectFromDb()
{
    Db* db = getSelectedDb();
    if (!db)
        return;

    if (!db->isOpen())
        return;

    db->close();
}

void DbTree::addTable()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    newTable(item);
}

void DbTree::editTable()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
    {
        qWarning() << "Tried to edit table, while table wasn't selected in DbTree.";
        return;
    }

    openTable(db, QString::null, table); // TODO put database name when supported
}

void DbTree::delTable()
{
    delSelectedObject();
}

void DbTree::addIndex()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();

    DbObjectDialogs dialogs(db);
    dialogs.addIndex(table);
}

void DbTree::editIndex()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString index = item->getIndex();

    DbObjectDialogs dialogs(db);
    dialogs.editIndex(index);
}

void DbTree::delIndex()
{
    delSelectedObject();
}

void DbTree::addTrigger()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    QString view = item->getView();

    DbObjectDialogs dialogs(db);
    dialogs.addTrigger(table, view);
}

void DbTree::editTrigger()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString trigger = item->getTrigger();

    DbObjectDialogs dialogs(db);
    dialogs.editTrigger(trigger);
}

void DbTree::delTrigger()
{
    delSelectedObject();
}

void DbTree::addView()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    newView(item);
}

void DbTree::editView()
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString view = item->getView();
    if (view.isNull())
    {
        qWarning() << "Tried to edit view, while view wasn't selected in DbTree.";
        return;
    }

    openView(item);
}

void DbTree::delView()
{
    delSelectedObject();
}

void DbTree::editColumn()
{
    DbTreeItem* item = ui->treeView->currentItem();
    if (!item)
        return;

    editColumn(item);
}

void DbTree::editColumn(DbTreeItem* item)
{
    Db* db = getSelectedOpenDb();
    if (!db)
        return;

    if (item->getType() != DbTreeItem::Type::COLUMN)
        return;

    DbTreeItem* tableItem = item->findParentItem(DbTreeItem::Type::TABLE);
    if (!tableItem)
        return;

    TableWindow* tableWin = openTable(tableItem);
    tableWin->editColumn(item->text());
}

void DbTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    UNUSED(previous);
    updateActionsFor(treeModel->itemFromIndex(current));
}

void DbTree::deleteSelected()
{
    QModelIndexList idxList = ui->treeView->selectionModel()->selectedRows();
    QList<DbTreeItem*> items;
    foreach (const QModelIndex& idx, idxList)
        items << dynamic_cast<DbTreeItem*>(treeModel->itemFromIndex(idx));

    deleteItems(items);
}

void DbTree::deleteItems(const QList<DbTreeItem*>& itemsToDelete)
{
    QList<DbTreeItem*> items = itemsToDelete;

    filterUndeletableItems(items);
    filterItemsWithParentInList(items);

    // Warning user about items to be deleted
    static const QString itemTmp = "<img src=\"%1\"/> %2";

    QStringList toDelete;
    QStringList databasesToRemove;
    int groupItems = 0;
    foreach (DbTreeItem* item, items)
    {
        if (item->getType() == DbTreeItem::Type::DB)
            databasesToRemove << itemTmp.arg(ICON_PATH(item->getIconName())).arg(item->text());
        else
            toDelete << itemTmp.arg(ICON_PATH(item->getIconName())).arg(item->text());

        if (item->getType() == DbTreeItem::Type::DIR)
            groupItems++;
    }

    QStringList actions;
    if (toDelete.size() > 0)
        actions << tr("Following objects will be deleted: %1.").arg(toDelete.join(", "));

    if (databasesToRemove.size() > 0)
        actions << tr("Following databases will be removed from list: %1.").arg(databasesToRemove.join(", "));

    if (groupItems > 0)
        actions << tr("Remainig objects from deleted group will be moved in place where the group used to be.");

    QString msg = tr("%1<br><br>Are you sure you want to continue?").arg(actions.join("<br><br>"));

    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Delete objects"), msg);
    if (result != QMessageBox::Yes)
        return;

    // Deleting items
    foreach (DbTreeItem* item, items)
        deleteItem(item);
}

void DbTree::refreshSchemas()
{
    foreach (Db* db, DBLIST->getDbList())
        treeModel->refreshSchema(db);
}

void DbTree::refreshSchema()
{
    Db* db = getSelectedDb();
    refreshSchema(db);
}

void DbTree::updateActionsForCurrent()
{
    updateActionsFor(ui->treeView->currentItem());
}

void DbTree::setupDefShortcuts()
{
    setShortcutContext({CLEAR_FILTER, DEL_SELECTED}, Qt::WidgetWithChildrenShortcut);

    defShortcut(DEL_SELECTED, Qt::Key_Delete);
    defShortcut(CLEAR_FILTER, Qt::Key_Escape);
}

int qHash(DbTree::Action action)
{
    return static_cast<int>(action);
}
