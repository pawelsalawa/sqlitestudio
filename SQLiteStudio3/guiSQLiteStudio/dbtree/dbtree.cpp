#include "dbtree.h"
#include "dbtreeitem.h"
#include "ui_dbtree.h"
#include "actionentry.h"
#include "common/utils_sql.h"
#include "dbtreemodel.h"
#include "dialogs/dbdialog.h"
#include "services/dbmanager.h"
#include "iconmanager.h"
#include "common/global.h"
#include "services/notifymanager.h"
#include "mainwindow.h"
#include "common/unused.h"
#include "dbobjectdialogs.h"
#include "common/userinputfilter.h"
#include "common/widgetcover.h"
#include "windows/tablewindow.h"
#include "dialogs/exportdialog.h"
#include "dialogs/importdialog.h"
#include "dialogs/populatedialog.h"
#include "services/importmanager.h"
#include "windows/editorwindow.h"
#include "uiconfig.h"
#include "themetuner.h"
#include "querygenerator.h"
#include "dialogs/execfromfiledialog.h"
#include "dialogs/fileexecerrorsdialog.h"
#include "common/compatibility.h"
#include "sqlfileexecutor.h"
#include "common/mouseshortcut.h"
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrentRun>

CFG_KEYS_DEFINE(DbTree)
QHash<DbTreeItem::Type,QList<DbTreeItem::Type>> DbTree::allowedTypesInside;
QSet<DbTreeItem::Type> DbTree::draggableTypes;

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

void DbTree::staticInit()
{
    initDndTypes();
}

void DbTree::init()
{
    ui->setupUi(this);
    initDndTypes();

    THEME_TUNER->manageCompactLayout(widget());

    fileExecutor = new SqlFileExecutor(this);

    ui->nameFilter->setClearButtonEnabled(true);

    treeRefreshWidgetCover = new WidgetCover(this);
    treeRefreshWidgetCover->initWithInterruptContainer();
    treeRefreshWidgetCover->hide();
    connect(treeRefreshWidgetCover, SIGNAL(cancelClicked()), this, SLOT(interrupt()));

    fileExecWidgetCover = new WidgetCover(this);
    fileExecWidgetCover->initWithInterruptContainer();
    fileExecWidgetCover->displayProgress(100);
    fileExecWidgetCover->hide();
    connect(fileExecWidgetCover, &WidgetCover::cancelClicked, fileExecutor, &SqlFileExecutor::stopExecution);
    connect(fileExecutor, SIGNAL(updateProgress(int)), this, SLOT(setFileExecProgress(int)), Qt::QueuedConnection);
    connect(fileExecutor, SIGNAL(execEnded()), this, SLOT(hideFileExecCover()), Qt::QueuedConnection);
    connect(fileExecutor, SIGNAL(execErrors(QList<QPair<QString, QString>>, bool)), this, SLOT(showFileExecErrors(QList<QPair<QString, QString>>, bool)),
            Qt::QueuedConnection);
    connect(fileExecutor, SIGNAL(schemaNeedsRefreshing(Db*)), this, SLOT(refreshSchema(Db*)), Qt::QueuedConnection);

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
    connect(DBLIST, SIGNAL(dbConnected(Db*)), this, SLOT(dbConnected(Db*)));
    connect(DBLIST, SIGNAL(dbDisconnected(Db*)), this, SLOT(dbDisconnected(Db*)));
    connect(IMPORT_MANAGER, SIGNAL(schemaModified(Db*)), this, SLOT(refreshSchema(Db*)));

    connect(CFG_UI.Fonts.DbTree, SIGNAL(changed(QVariant)), this, SLOT(refreshFont()));
    MouseShortcut::forWheel(Qt::ControlModifier, this, SLOT(fontSizeChangeRequested(int)), ui->treeView->viewport());

    connect(treeModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(sessionValueChanged()));
    connect(treeModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SIGNAL(sessionValueChanged()));
    connect(treeModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(sessionValueChanged()));
    connect(ui->treeView, SIGNAL(expanded(QModelIndex)), this, SIGNAL(sessionValueChanged()));
    connect(ui->treeView, SIGNAL(collapsed(QModelIndex)), this, SIGNAL(sessionValueChanged()));

    updateActionsForCurrent();
}

void DbTree::createActions()
{
    createAction(COPY, ICONS.ACT_COPY, tr("Copy"), this, SLOT(copy()), this);
    createAction(PASTE, ICONS.ACT_PASTE, tr("Paste"), this, SLOT(paste()), this);
    createAction(SELECT_ALL, ICONS.ACT_SELECT_ALL, tr("Select all"), this, SLOT(selectAll()), this);
    createAction(CREATE_GROUP, ICONS.DIRECTORY_ADD, tr("Create a group"), this, SLOT(createGroup()), this);
    createAction(DELETE_GROUP, ICONS.DIRECTORY_DEL, tr("Delete the group"), this, SLOT(deleteGroup()), this);
    createAction(RENAME_GROUP, ICONS.DIRECTORY_EDIT, tr("Rename the group"), this, SLOT(renameGroup()), this);
    createAction(ADD_DB, ICONS.DATABASE_ADD, tr("&Add a database"), this, SLOT(addDb()), this);
    createAction(EDIT_DB, ICONS.DATABASE_EDIT, tr("&Edit the database"), this, SLOT(editDb()), this);
    createAction(DELETE_DB, ICONS.DATABASE_DEL, tr("&Remove the database"), this, SLOT(removeDb()), this);
    createAction(CONNECT_TO_DB, ICONS.DATABASE_CONNECT, tr("&Connect to the database"), this, SLOT(connectToDb()), this);
    createAction(DISCONNECT_FROM_DB, ICONS.DATABASE_DISCONNECT, tr("&Disconnect from the database"), this, SLOT(disconnectFromDb()), this);
    createAction(IMPORT_INTO_DB, ICONS.IMPORT, tr("Import"), this, SLOT(import()), this);
    createAction(EXPORT_DB, ICONS.DATABASE_EXPORT, tr("&Export the database"), this, SLOT(exportDb()), this);
    createAction(VACUUM_DB, ICONS.VACUUM_DB, tr("Vac&uum"), this, SLOT(vacuumDb()), this);
    createAction(INTEGRITY_CHECK, ICONS.INTEGRITY_CHECK, tr("&Integrity check"), this, SLOT(integrityCheck()), this);
    createAction(ADD_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), this, SLOT(addTable()), this);
    createAction(EDIT_TABLE, ICONS.TABLE_EDIT, tr("Edit the t&able"), this, SLOT(editTable()), this);
    createAction(DEL_TABLE, ICONS.TABLE_DEL, tr("Delete the ta&ble"), this, SLOT(delTable()), this);
    createAction(EXPORT_TABLE, ICONS.TABLE_EXPORT, tr("Export the table"), this, SLOT(exportTable()), this);
    createAction(IMPORT_TABLE, ICONS.TABLE_IMPORT, tr("Import into the table"), this, SLOT(importTable()), this);
    createAction(POPULATE_TABLE, ICONS.TABLE_POPULATE, tr("Populate table"), this, SLOT(populateTable()), this);
    createAction(CREATE_SIMILAR_TABLE, ICONS.TABLE_CREATE_SIMILAR, tr("Create similar table"), this, SLOT(createSimilarTable()), this);
    createAction(RESET_AUTOINCREMENT, ICONS.RESET_AUTOINCREMENT, tr("Reset autoincrement sequence"), this, SLOT(resetAutoincrement()), this);
    createAction(ADD_INDEX, ICONS.INDEX_ADD, tr("Create an &index"), this, SLOT(addIndex()), this);
    createAction(EDIT_INDEX, ICONS.INDEX_EDIT, tr("Edit the i&ndex"), this, SLOT(editIndex()), this);
    createAction(DEL_INDEX, ICONS.INDEX_DEL, tr("Delete the in&dex"), this, SLOT(delIndex()), this);
    createAction(ADD_TRIGGER, ICONS.TRIGGER_ADD, tr("Create a trig&ger"), this, SLOT(addTrigger()), this);
    createAction(EDIT_TRIGGER, ICONS.TRIGGER_EDIT, tr("Edit the trigg&er"), this, SLOT(editTrigger()), this);
    createAction(DEL_TRIGGER, ICONS.TRIGGER_DEL, tr("Delete the trigge&r"), this, SLOT(delTrigger()), this);
    createAction(ADD_VIEW, ICONS.VIEW_ADD, tr("Create a &view"), this, SLOT(addView()), this);
    createAction(EDIT_VIEW, ICONS.VIEW_EDIT, tr("Edit the v&iew"), this, SLOT(editView()), this);
    createAction(DEL_VIEW, ICONS.VIEW_DEL, tr("Delete the vi&ew"), this, SLOT(delView()), this);
    createAction(ADD_COLUMN, ICONS.TABLE_COLUMN_ADD, tr("Add a column"), this, SLOT(addColumn()), this);
    createAction(EDIT_COLUMN, ICONS.TABLE_COLUMN_EDIT, tr("Edit the column"), this, SLOT(editColumn()), this);
    createAction(DEL_COLUMN, ICONS.TABLE_COLUMN_DELETE, tr("Delete the column"), this, SLOT(delColumn()), this);
    createAction(DEL_SELECTED, ICONS.DELETE_SELECTED, tr("Delete selected items"), this, SLOT(deleteSelected()), this);
    createAction(CLEAR_FILTER, tr("Clear filter"), ui->nameFilter, SLOT(clear()), this);
    createAction(REFRESH_SCHEMAS, ICONS.DATABASE_RELOAD, tr("&Refresh all database schemas"), this, SLOT(refreshSchemas()), this);
    createAction(REFRESH_SCHEMA, ICONS.DATABASE_RELOAD, tr("Re&fresh selected database schema"), this, SLOT(refreshSchema()), this);
    createAction(ERASE_TABLE_DATA, ICONS.ERASE_TABLE_DATA, tr("Erase table data"), this, SLOT(eraseTableData()), this);
    createAction(GENERATE_SELECT, "SELECT", this, SLOT(generateSelectForTable()), this);
    createAction(GENERATE_INSERT, "INSERT", this, SLOT(generateInsertForTable()), this);
    createAction(GENERATE_UPDATE, "UPDATE", this, SLOT(generateUpdateForTable()), this);
    createAction(GENERATE_DELETE, "DELETE", this, SLOT(generateDeleteForTable()), this);
    createAction(OPEN_DB_DIRECTORY, ICONS.DIRECTORY_OPEN_WITH_DB, tr("Open file's directory"), this, SLOT(openDbDirectory()), this);
    createAction(EXEC_SQL_FROM_FILE, ICONS.EXEC_SQL_FROM_FILE, tr("Execute SQL from file"), this, SLOT(execSqlFromFile()), this);
    createAction(INCR_FONT_SIZE, tr("Increase font size", "database list"), this, SLOT(incrFontSize()), this);
    createAction(DECR_FONT_SIZE, tr("Decrease font size", "database list"), this, SLOT(decrFontSize()), this);
}

void DbTree::updateActionStates(const QStandardItem *item)
{
    QList<int> enabled;
    const DbTreeItem* dbTreeItem = dynamic_cast<const DbTreeItem*>(item);
    if (item != nullptr)
    {
        bool isDbOpen = false;
        DbTreeItem* parentItem = dbTreeItem->parentDbTreeItem();
        DbTreeItem* grandParentItem = parentItem ? parentItem->parentDbTreeItem() : nullptr;

        // Add database should always be available, as well as a copy of an item
        enabled << ADD_DB << COPY;

        if (isMimeDataValidForItem(QApplication::clipboard()->mimeData(), dbTreeItem, true))
            enabled << PASTE;

        enabled << CLEAR_FILTER;

        // Group actions
        if (dbTreeItem->getType() == DbTreeItem::Type::DIR)
            enabled << CREATE_GROUP << RENAME_GROUP << DELETE_GROUP << ADD_DB;

        if (dbTreeItem->getDb())
        {
            enabled << DELETE_DB << EDIT_DB;
            if (dbTreeItem->getDb()->isOpen())
            {
                enabled << DISCONNECT_FROM_DB << IMPORT_INTO_DB << EXPORT_DB << REFRESH_SCHEMA
                        << VACUUM_DB << INTEGRITY_CHECK;
                isDbOpen = true;
            }
            else
                enabled << CONNECT_TO_DB;

            QUrl url = QUrl::fromLocalFile(dbTreeItem->getDb()->getPath());
            if (url.isValid())
                enabled << OPEN_DB_DIRECTORY;
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
                    enabled << CREATE_GROUP << DELETE_DB << EDIT_DB << EXEC_SQL_FROM_FILE;
                    break;
                case DbTreeItem::Type::TABLES:
                    break;
                case DbTreeItem::Type::TABLE:
                    enabled << EDIT_TABLE << DEL_TABLE << EXPORT_TABLE << IMPORT_TABLE << POPULATE_TABLE << ADD_COLUMN << CREATE_SIMILAR_TABLE;
                    enabled << RESET_AUTOINCREMENT << ADD_INDEX << ADD_TRIGGER << ERASE_TABLE_DATA;
                    enabled << GENERATE_SELECT << GENERATE_INSERT << GENERATE_UPDATE << GENERATE_DELETE;
                    break;
                case DbTreeItem::Type::VIRTUAL_TABLE:
                    // TODO change below when virtual tables can be edited
//                    enabled << EDIT_TABLE << DEL_TABLE;
                    enabled << DEL_TABLE;
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
                    if (parentItem && parentItem->getType() == DbTreeItem::Type::TABLE)
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
                    if (grandParentItem && grandParentItem->getType() == DbTreeItem::Type::TABLE)
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
                    enabled << EDIT_TABLE << DEL_TABLE << EXPORT_TABLE << IMPORT_TABLE << POPULATE_TABLE << ADD_COLUMN;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
                case DbTreeItem::Type::COLUMN:
                    enabled << EDIT_TABLE << DEL_TABLE << EXPORT_TABLE << IMPORT_TABLE << POPULATE_TABLE << ADD_COLUMN << DEL_COLUMN;
                    enabled << EDIT_COLUMN;
                    enabled << ADD_INDEX << ADD_TRIGGER;
                    break;
            }
        }

        // Do we have any deletable object selected? If yes, enable "Del" action.
        bool enableDel = false;
        for (DbTreeItem* selItem : getModel()->getItemsForIndexes(getView()->getSelectedIndexes()))
        {
            switch (selItem->getType())
            {
                case DbTreeItem::Type::COLUMN:
                case DbTreeItem::Type::DB:
                case DbTreeItem::Type::DIR:
                case DbTreeItem::Type::INDEX:
                case DbTreeItem::Type::TABLE:
                case DbTreeItem::Type::TRIGGER:
                case DbTreeItem::Type::VIEW:
                case DbTreeItem::Type::VIRTUAL_TABLE:
                    enableDel = true;
                    break;
                case DbTreeItem::Type::COLUMNS:
                case DbTreeItem::Type::INDEXES:
                case DbTreeItem::Type::ITEM_PROTOTYPE:
                case DbTreeItem::Type::TABLES:
                case DbTreeItem::Type::TRIGGERS:
                case DbTreeItem::Type::VIEWS:
                    break;
            }

            if (enableDel)
            {
                enabled  << DEL_SELECTED;
                break;
            }
        }
    }
    else
    {
        enabled << CREATE_GROUP << ADD_DB;
    }

    if (treeModel->rowCount() > 0)
    {
        enabled << SELECT_ALL; // if there's at least 1 item, enable this

        // Table/view always enabled, as long as there is at least 1 db on the list. #4017
        if (treeModel->findFirstItemOfType(DbTreeItem::Type::DB))
            enabled << ADD_TABLE << ADD_VIEW;
    }

    enabled << REFRESH_SCHEMAS;

    for (int action : actionMap.keys())
        setActionEnabled(action, enabled.contains(action));
}

void DbTree::setupActionsForMenu(DbTreeItem* currItem, QMenu* contextMenu)
{
    QList<ActionEntry> actions;

    ActionEntry dbEntry(ICONS.DATABASE, tr("Database"));
    dbEntry += ADD_DB;
    dbEntry += EDIT_DB;
    dbEntry += DELETE_DB;

    ActionEntry dbEntryExt(ICONS.DATABASE, tr("Database"));
    dbEntryExt += CONNECT_TO_DB;
    dbEntryExt += DISCONNECT_FROM_DB;
    dbEntryExt += _separator;
    dbEntryExt += REFRESH_SCHEMA;
    dbEntryExt += _separator;
    dbEntryExt += ADD_DB;
    dbEntryExt += EDIT_DB;
    dbEntryExt += DELETE_DB;

    ActionEntry groupEntry(ICONS.DIRECTORY, tr("Grouping"));
    groupEntry += CREATE_GROUP;
    groupEntry += RENAME_GROUP;
    groupEntry += DELETE_GROUP;

    ActionEntry genQueryEntry(ICONS.GENERATE_QUERY, tr("Generate query for table"));
    genQueryEntry += GENERATE_SELECT;
    genQueryEntry += GENERATE_INSERT;
    genQueryEntry += GENERATE_UPDATE;
    genQueryEntry += GENERATE_DELETE;

    if (currItem)
    {
        DbTreeItem* parentItem = currItem->parentDbTreeItem();
        DbTreeItem* grandParentItem = parentItem ? parentItem->parentDbTreeItem() : nullptr;
        DbTreeItem::Type itemType = currItem->getType();
        switch (itemType)
        {
            case DbTreeItem::Type::DIR:
            {
                actions += ActionEntry(CREATE_GROUP);
                actions += ActionEntry(RENAME_GROUP);
                actions += ActionEntry(DELETE_GROUP);
                actions += ActionEntry(_separator);
                actions += dbEntry;
                break;
            }
            case DbTreeItem::Type::DB:
            {
                if (currItem->getDb()->isValid())
                {
                    actions += ActionEntry(CONNECT_TO_DB);
                    actions += ActionEntry(DISCONNECT_FROM_DB);
                    actions += ActionEntry(_separator);
                    actions += ActionEntry(ADD_DB);
                    actions += ActionEntry(EDIT_DB);
                    actions += ActionEntry(DELETE_DB);
                    actions += ActionEntry(_separator);
                    actions += ActionEntry(ADD_TABLE);
                    actions += ActionEntry(ADD_INDEX);
                    actions += ActionEntry(ADD_TRIGGER);
                    actions += ActionEntry(ADD_VIEW);
                    actions += ActionEntry(_separator);
                    actions += ActionEntry(REFRESH_SCHEMA);
                    actions += ActionEntry(IMPORT_INTO_DB);
                    actions += ActionEntry(EXPORT_DB);
                    actions += ActionEntry(VACUUM_DB);
                    actions += ActionEntry(INTEGRITY_CHECK);
                    actions += ActionEntry(EXEC_SQL_FROM_FILE);
                    actions += ActionEntry(OPEN_DB_DIRECTORY);
                    actions += ActionEntry(_separator);
                }
                else
                {
                    actions += ActionEntry(ADD_DB);
                    actions += ActionEntry(EDIT_DB);
                    actions += ActionEntry(DELETE_DB);
                    actions += ActionEntry(_separator);
                }
                break;
            }
            case DbTreeItem::Type::TABLES:
                actions += ActionEntry(ADD_TABLE);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::TABLE:
                actions += ActionEntry(ADD_TABLE);
                actions += ActionEntry(EDIT_TABLE);
                actions += ActionEntry(DEL_TABLE);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_COLUMN);
                actions += ActionEntry(ADD_INDEX);
                actions += ActionEntry(ADD_TRIGGER);
                actions += ActionEntry(_separator);
                actions += genQueryEntry;
                actions += ActionEntry(IMPORT_TABLE);
                actions += ActionEntry(EXPORT_TABLE);
                actions += ActionEntry(POPULATE_TABLE);
                actions += ActionEntry(CREATE_SIMILAR_TABLE);
                actions += ActionEntry(RESET_AUTOINCREMENT);
                actions += ActionEntry(ERASE_TABLE_DATA);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::VIRTUAL_TABLE:
                actions += ActionEntry(ADD_TABLE);
                //actions += ActionEntry(EDIT_TABLE); // TODO uncomment when virtual tables have their own edition window
                actions += ActionEntry(DEL_TABLE);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::INDEXES:
                actions += ActionEntry(ADD_INDEX);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_TABLE);
                actions += ActionEntry(EDIT_TABLE);
                actions += ActionEntry(DEL_TABLE);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::INDEX:
                actions += ActionEntry(ADD_INDEX);
                actions += ActionEntry(EDIT_INDEX);
                actions += ActionEntry(DEL_INDEX);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_TABLE);
                actions += ActionEntry(EDIT_TABLE);
                actions += ActionEntry(DEL_TABLE);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::TRIGGERS:
            {
                actions += ActionEntry(ADD_TRIGGER);
                actions += ActionEntry(_separator);
                if (parentItem && parentItem->getType() == DbTreeItem::Type::TABLE)
                {
                    actions += ActionEntry(ADD_TABLE);
                    actions += ActionEntry(EDIT_TABLE);
                    actions += ActionEntry(DEL_TABLE);
                }
                else
                {
                    actions += ActionEntry(ADD_VIEW);
                    actions += ActionEntry(EDIT_VIEW);
                    actions += ActionEntry(DEL_VIEW);
                }
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            }
            case DbTreeItem::Type::TRIGGER:
            {
                actions += ActionEntry(ADD_TRIGGER);
                actions += ActionEntry(EDIT_TRIGGER);
                actions += ActionEntry(DEL_TRIGGER);
                actions += ActionEntry(_separator);
                if (grandParentItem && grandParentItem->getType() == DbTreeItem::Type::TABLE)
                {
                    actions += ActionEntry(ADD_TABLE);
                    actions += ActionEntry(EDIT_TABLE);
                    actions += ActionEntry(DEL_TABLE);
                }
                else
                {
                    actions += ActionEntry(ADD_VIEW);
                    actions += ActionEntry(EDIT_VIEW);
                    actions += ActionEntry(DEL_VIEW);
                }
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            }
            case DbTreeItem::Type::VIEWS:
                actions += ActionEntry(ADD_VIEW);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::VIEW:
                actions += ActionEntry(ADD_VIEW);
                actions += ActionEntry(EDIT_VIEW);
                actions += ActionEntry(DEL_VIEW);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_TRIGGER);
                actions += ActionEntry(EDIT_TRIGGER);
                actions += ActionEntry(DEL_TRIGGER);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::COLUMNS:
                actions += ActionEntry(ADD_COLUMN);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_TABLE);
                actions += ActionEntry(EDIT_TABLE);
                actions += ActionEntry(DEL_TABLE);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_INDEX);
                actions += ActionEntry(ADD_TRIGGER);
                actions += ActionEntry(_separator);
                actions += ActionEntry(IMPORT_TABLE);
                actions += ActionEntry(EXPORT_TABLE);
                actions += ActionEntry(POPULATE_TABLE);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::COLUMN:
                actions += ActionEntry(ADD_COLUMN);
                actions += ActionEntry(EDIT_COLUMN);
                actions += ActionEntry(DEL_COLUMN);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_TABLE);
                actions += ActionEntry(EDIT_TABLE);
                actions += ActionEntry(DEL_TABLE);
                actions += ActionEntry(_separator);
                actions += ActionEntry(ADD_INDEX);
                actions += ActionEntry(ADD_TRIGGER);
                actions += ActionEntry(_separator);
                actions += ActionEntry(IMPORT_TABLE);
                actions += ActionEntry(EXPORT_TABLE);
                actions += ActionEntry(POPULATE_TABLE);
                actions += ActionEntry(_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::ITEM_PROTOTYPE:
                break;
        }

        actions += ActionEntry(_separator);

        if (itemType == DbTreeItem::Type::DB)
            actions += groupEntry;
    }
    else
    {
        actions += dbEntry;
        actions += ActionEntry(_separator);
        actions += groupEntry;
    }

    actions += COPY;
    actions += PASTE;
    actions += _separator;
    actions += DEL_SELECTED;
    actions += _separator;
    actions += SELECT_ALL;
    actions += ActionEntry(REFRESH_SCHEMAS);

    QMenu* subMenu = nullptr;
    for (ActionEntry actionEntry : actions)
    {
        switch (actionEntry.type)
        {
            case ActionEntry::Type::SINGLE:
            {
                if (actionEntry.action == DbTree::_separator)
                {
                    contextMenu->addSeparator();
                    break;
                }
                contextMenu->addAction(actionMap[actionEntry.action]);
                break;
            }
            case ActionEntry::Type::SUB_MENU:
            {
                subMenu = contextMenu->addMenu(actionEntry.subMenuIcon, actionEntry.subMenuLabel);
                for (Action action : actionEntry.actions)
                {
                    if (action == DbTree::_separator)
                    {
                        subMenu->addSeparator();
                        continue;
                    }
                    subMenu->addAction(actionMap[action]);
                }
                break;
            }
        }
    }
}

void DbTree::initDndTypes()
{
    draggableTypes << DbTreeItem::Type::TABLE << DbTreeItem::Type::VIEW << DbTreeItem::Type::DIR << DbTreeItem::Type::DB;

    allowedTypesInside[DbTreeItem::Type::DIR] << DbTreeItem::Type::DB << DbTreeItem::Type::DIR;
    allowedTypesInside[DbTreeItem::Type::DB] << DbTreeItem::Type::TABLE << DbTreeItem::Type::VIEW;
    allowedTypesInside[DbTreeItem::Type::TABLES] << DbTreeItem::Type::TABLE << DbTreeItem::Type::VIEW;
    allowedTypesInside[DbTreeItem::Type::TABLE] << DbTreeItem::Type::TABLE << DbTreeItem::Type::VIEW;
    allowedTypesInside[DbTreeItem::Type::VIEWS] << DbTreeItem::Type::TABLE << DbTreeItem::Type::VIEW;
    allowedTypesInside[DbTreeItem::Type::VIEW] << DbTreeItem::Type::TABLE << DbTreeItem::Type::VIEW;
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

DbTreeView*DbTree::getView() const
{
    return ui->treeView;
}

bool DbTree::isMimeDataValidForItem(const QMimeData* mimeData, const DbTreeItem* item, bool forPasting)
{
    if (mimeData->formats().contains(DbTreeModel::MIMETYPE))
        return areDbTreeItemsValidForItem(getModel()->getDragItems(mimeData), item, forPasting);
    else if (mimeData->hasUrls())
        return areUrlsValidForItem(mimeData->urls(), item);

    return false;
}

bool DbTree::isItemDraggable(const DbTreeItem* item)
{
    return item && draggableTypes.contains(item->getType());
}

bool DbTree::areDbTreeItemsValidForItem(QList<DbTreeItem*> srcItems, const DbTreeItem* dstItem, bool forPasting)
{
    QSet<Db*> srcDbs;
    QList<DbTreeItem::Type> srcTypes;
    DbTreeItem::Type dstType = DbTreeItem::Type::DIR; // the empty space is treated as group
    if (dstItem)
        dstType = dstItem->getType();

    if (dstType == DbTreeItem::Type::DB && !dstItem->getDb()->isOpen())
        return false;

    for (DbTreeItem* srcItem : srcItems)
    {
        if (!srcItem)
        {
            srcTypes << DbTreeItem::Type::ITEM_PROTOTYPE;
            continue;
        }

        srcTypes << srcItem->getType();
        if (srcItem->getDb())
            srcDbs << srcItem->getDb();
    }

    for (DbTreeItem::Type& srcType : srcTypes)
    {
        if (!allowedTypesInside[dstType].contains(srcType))
            return false;
    }

    // Support for d&d reordering of db objects
    static const QHash<DbTreeItem::Type, DbTreeItem::Type> reorderingTypeToParent = {
        {DbTreeItem::Type::TABLE, DbTreeItem::Type::TABLES},
        {DbTreeItem::Type::TRIGGER, DbTreeItem::Type::TRIGGERS},
        {DbTreeItem::Type::VIEW, DbTreeItem::Type::VIEWS},
        {DbTreeItem::Type::INDEX, DbTreeItem::Type::INDEXES}
    };

    if (!forPasting && toSet(srcTypes).size() == 1 && srcDbs.size() == 1 && dstItem &&
            *(srcDbs.begin()) == dstItem->getDb() && reorderingTypeToParent[srcTypes.first()] == dstType)
        return true;

    // No other d&d within same db
    if (dstItem && dstItem->getDb() && srcDbs.contains(dstItem->getDb()))
        return false;

    return true;
}

bool DbTree::areUrlsValidForItem(const QList<QUrl>& srcUrls, const DbTreeItem* dstItem)
{
    UNUSED(dstItem);
    for (const QUrl& srcUrl : srcUrls)
    {
        if (!srcUrl.isLocalFile())
            return false;
    }
    return true;
}

void DbTree::showRefreshWidgetCover()
{
    treeRefreshWidgetCover->show();
}

void DbTree::hideRefreshWidgetCover()
{
    treeRefreshWidgetCover->hide();
}

void DbTree::setSelectedItem(DbTreeItem *item)
{
    ui->treeView->setCurrentIndex(item->index());
    ui->treeView->selectionModel()->select(item->index(), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

QToolBar* DbTree::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
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

QSet<Db*> DbTree::getSelectedDatabases()
{
    QList<DbTreeItem*> items = ui->treeView->selectionItems();
    QSet<Db*> dbList;
    for (DbTreeItem* item : items)
        dbList << item->getDb();

    return dbList;
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
    QString database = QString(); // TODO implement this when named databases (attached) are handled by dbtree.
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
    //QString database = QString(); // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();

    DbObjectDialogs dialogs(db);
    dialogs.editIndex(item->text());
}

ViewWindow* DbTree::openView(DbTreeItem* item)
{
    QString database = QString(); // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();
    return openView(db, database, item->text());
}

ViewWindow* DbTree::openView(Db* db, const QString& database, const QString& view)
{
    DbObjectDialogs dialogs(db);
    return dialogs.editView(database, view);
}

TableWindow* DbTree::newTable(Db* db)
{
    DbObjectDialogs dialogs(db);
    return dialogs.addTable();
}

ViewWindow* DbTree::newView(Db* db)
{
    DbObjectDialogs dialogs(db);
    return dialogs.addView();
}

void DbTree::editTrigger(DbTreeItem* item)
{
    //QString database = QString(); // TODO implement this when named databases (attached) are handled by dbtree.
    Db* db = item->getDb();

    DbObjectDialogs dialogs(db);
    dialogs.editTrigger(item->text());
}

//void DbTree::delSelectedObject()
//{
//    Db* db = getSelectedOpenDb();
//    if (!db)
//        return;

//    DbTreeItem* item = ui->treeView->currentItem();
//    if (!item)
//        return;

//    DbObjectDialogs dialogs(db);
//    dialogs.dropObject(item->text()); // TODO add database prefix when supported
//}

void DbTree::filterUndeletableItems(QList<DbTreeItem*>& items)
{
    QMutableListIterator<DbTreeItem*> it(items);
    while (it.hasNext())
    {
        DbTreeItem::Type type = it.next()->getType();
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
            case DbTreeItem::Type::TABLE:
            case DbTreeItem::Type::VIRTUAL_TABLE:
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
    DbTreeItem* item = nullptr;
    while (it.hasNext())
    {
        item = it.next();
        for (DbTreeItem* pathItem : item->getPathToRoot().mid(1))
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
    DbObjectDialogs::Type objType = DbObjectDialogs::Type::UNKNOWN;
    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
            treeModel->deleteGroup(item);
            return;
        case DbTreeItem::Type::DB:
            DBLIST->removeDb(item->getDb());
            return;
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::VIRTUAL_TABLE:
            objType = DbObjectDialogs::Type::TABLE;
            break;
        case DbTreeItem::Type::INDEX:
            objType = DbObjectDialogs::Type::INDEX;
            break;
        case DbTreeItem::Type::TRIGGER:
            objType = DbObjectDialogs::Type::TRIGGER;
            break;
        case DbTreeItem::Type::VIEW:
            objType = DbObjectDialogs::Type::VIEW;
            break;
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::VIEWS:
        case DbTreeItem::Type::COLUMNS:
        case DbTreeItem::Type::COLUMN:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            return;
    }

    if (objType != DbObjectDialogs::Type::UNKNOWN)
    {
        Db* db = item->getDb();
        DbObjectDialogs dialogs(db);
        dialogs.setNoConfirmation(true); // confirmation is done in deleteSelected()
        dialogs.setNoSchemaRefreshing(true); // we will refresh after all items are deleted
        dialogs.dropObject(objType, item->text()); // TODO database name when supported
    }
}

void DbTree::deleteSelected(DbTreeItem::Type itemType)
{
    deleteSelected([itemType](DbTreeItem* item)
    {
        return item->getType() == itemType;
    });
}

QHash<Db*, QList<DbTreeItem*>> DbTree::groupByDb(const QList<DbTreeItem*> items)
{
    QHash<Db*, QList<DbTreeItem*>> grouped;
    for (DbTreeItem* item : items)
        grouped[item->getDb()] << item;

    return grouped;
}

QStringList DbTree::itemsToNames(const QList<DbTreeItem*>& items)
{
    QStringList names;
    for (DbTreeItem* item : items)
        names << item->text();

    return names;
}

QString DbTree::getSelectedTableName() const
{
    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
        return QString();

    return table;
}

QString DbTree::getSelectedIndexName() const
{
    DbTreeItem* item = ui->treeView->currentItem();
    QString idx = item->getIndex();
    if (idx.isNull())
        return QString();

    return idx;
}

QString DbTree::getSelectedTriggerName() const
{
    DbTreeItem* item = ui->treeView->currentItem();
    QString trig = item->getTrigger();
    if (trig.isNull())
        return QString();

    return trig;
}

QString DbTree::getSelectedViewName() const
{
    DbTreeItem* item = ui->treeView->currentItem();
    QString view= item->getView();
    if (view.isNull())
        return QString();

    return view;
}

QList<DbTreeItem *> DbTree::getSelectedItems(DbTreeItem::Type itemType)
{
    return getSelectedItems([itemType](DbTreeItem* item)
    {
        return item->getType() == itemType;
    });
}

void DbTree::refreshSchema(Db* db)
{
    if (!db)
        return;

    if (!db->isOpen())
        return;

    treeModel->refreshSchema(db);
    updateActionsForCurrent();

    for (MdiChild*& mdi : MAINWINDOW->getMdiArea()->getMdiChilds())
    {
        EditorWindow* editor = dynamic_cast<EditorWindow*>(mdi);
        if (!editor)
            continue;

        Db* editorDb = editor->getCurrentDb();
        if (!editorDb || editorDb != db)
            continue;

        editor->refreshValidDbObjects();
    }
}

void DbTree::copy()
{
    QMimeData* mimeData = treeModel->mimeData(ui->treeView->getSelectedIndexes());
    QApplication::clipboard()->setMimeData(mimeData);
}

void DbTree::paste()
{
    DbTreeItem* currItem = ui->treeView->currentItem();
    QModelIndex idx;
    if (currItem)
        idx = currItem->index();

    treeModel->pasteData(QApplication::clipboard()->mimeData(), -1, -1, idx, Qt::CopyAction);
}

void DbTree::selectAll()
{
    ui->treeView->selectAll();
}

void DbTree::createGroup()
{
    QStringList existingItems;
    QStandardItem* currItem = ui->treeView->getItemForAction(true);
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
        QMessageBox::information(this, tr("Create group"), tr("Entry with name %1 already exists in group %2.")
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
            tr("Are you sure you want to delete group %1?\nAll objects from this group will be moved to parent group.").arg(item->text().left(ITEM_TEXT_LIMIT)));

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

    bool perm = CFG->isDbInConfig(db->getName());

    DbDialog dialog(DbDialog::EDIT, this);
    dialog.setDb(db);
    dialog.setPermanent(perm);
    dialog.exec();
}

void DbTree::removeDb()
{
    QList<Db*> dbList = getSelectedDatabases().values();
    if (dbList.isEmpty())
        return;

    QString msg;
    if (dbList.size() == 1)
    {
        msg = tr("Are you sure you want to remove database '%1' from the list?").arg(dbList.first()->getName().left(ITEM_TEXT_LIMIT));
    }
    else
    {
        QStringList dbNames;
        for (Db* db : dbList)
            dbNames << db->getName().left(ITEM_TEXT_LIMIT);

        msg = tr("Are you sure you want to remove following databases from the list:\n%1").arg(dbNames.join(",\n"));
    }
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Remove database"), msg);
    if (result != QMessageBox::Yes)
        return;

    for (Db* db : dbList)
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

    DbTreeItem* dbItem = ui->treeView->currentDbItem();
    ui->treeView->setCurrentIndex(dbItem->index());

    db->close();
}


void DbTree::import()
{
    if (!ImportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot import, because no import plugin is loaded."));
        return;
    }

    ImportDialog dialog(this);
    Db* db = getSelectedDb();
    if (db)
        dialog.setDb(db);

    dialog.exec();
}

void DbTree::exportDb()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    if (!ExportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot export, because no export plugin is loaded."));
        return;
    }

    ExportDialog dialog(this);
    dialog.setDatabaseMode(db);
    dialog.exec();
}

void DbTree::addTable()
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
    {
        DbTreeItem* item = treeModel->findFirstItemOfType(DbTreeItem::Type::DB);
        if (item)
            db = item->getDb();
    }

    if (!db || !db->isValid())
        return;

    newTable(db);
}

void DbTree::editTable()
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    QString table = getSelectedTableName();
    if (table.isNull())
    {
        qWarning() << "Tried to edit table, while table wasn't selected in DbTree.";
        return;
    }

    openTable(db, QString(), table); // TODO put database name when supported
}

void DbTree::delTable()
{
    deleteSelected(DbTreeItem::Type::TABLE);
}

void DbTree::addIndex()
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();

    DbObjectDialogs dialogs(db);
    dialogs.addIndex(table);
}

void DbTree::editIndex()
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    QString index = getSelectedIndexName();

    DbObjectDialogs dialogs(db);
    dialogs.editIndex(index);
}

void DbTree::delIndex()
{
    deleteSelected(DbTreeItem::Type::INDEX);
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
    if (!db || !db->isValid())
        return;

    QString trigger = getSelectedTriggerName();

    DbObjectDialogs dialogs(db);
    dialogs.editTrigger(trigger);
}

void DbTree::delTrigger()
{
    deleteSelected(DbTreeItem::Type::TRIGGER);
}

void DbTree::addView()
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
    {
        DbTreeItem* item = treeModel->findFirstItemOfType(DbTreeItem::Type::DB);
        if (item)
            db = item->getDb();
    }

    if (!db || !db->isValid())
        return;

    newView(db);
}

void DbTree::editView()
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    QString view = getSelectedViewName();
    if (view.isNull())
    {
        qWarning() << "Tried to edit view, while view wasn't selected in DbTree.";
        return;
    }

    openView(db, QString(), view); // TODO handle named database when supported
}

void DbTree::delView()
{
    deleteSelected(DbTreeItem::Type::VIEW);
}

void DbTree::exportTable()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
    {
        qWarning() << "Tried to export table, while table wasn't selected in DbTree.";
        return;
    }

    if (!ExportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot export, because no export plugin is loaded."));
        return;
    }

    ExportDialog dialog(this);
    dialog.setTableMode(db, table);
    dialog.exec();
}

void DbTree::importTable()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
    {
        qWarning() << "Tried to import into table, while table wasn't selected in DbTree.";
        return;
    }

    if (!ImportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot import, because no import plugin is loaded."));
        return;
    }

    ImportDialog dialog(this);
    dialog.setDbAndTable(db, table);
    dialog.exec();
}

void DbTree::populateTable()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
    {
        qWarning() << "Tried to populate table, while table wasn't selected in DbTree.";
        return;
    }

    PopulateDialog dialog(this);
    dialog.setDbAndTable(db, table);
    dialog.exec();
}

void DbTree::addColumn()
{
    DbTreeItem* item = ui->treeView->currentItem();
    if (!item)
        return;

    addColumn(item);
}

void DbTree::editColumn()
{
    DbTreeItem* item = ui->treeView->currentItem();
    if (!item)
        return;

    editColumn(item);
}

void DbTree::delColumn()
{
    DbTreeItem* item = ui->treeView->currentItem();
    if (!item)
        return;

    delColumn(item);
}

void DbTree::vacuumDb()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    EditorWindow* win = MAINWINDOW->openSqlEditor(db, "VACUUM;");
    if (!win)
        return;

    win->getMdiWindow()->rename(tr("Vacuum (%1)").arg(db->getName()));
    win->execute();
}

void DbTree::integrityCheck()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    EditorWindow* win = MAINWINDOW->openSqlEditor(db, "PRAGMA integrity_check;");
    if (!win)
        return;

    win->getMdiWindow()->rename(tr("Integrity check (%1)").arg(db->getName()));
    win->execute();
}

void DbTree::createSimilarTable()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
    {
        qWarning() << "Tried to clone table, while table wasn't selected in DbTree.";
        return;
    }

    DbObjectDialogs dialog(db);
    dialog.addTableSimilarTo(QString(), table);
}

void DbTree::resetAutoincrement()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* item = ui->treeView->currentItem();
    QString table = item->getTable();
    if (table.isNull())
    {
        qWarning() << "Tried to reset autoincrement, while table wasn't selected in DbTree.";
        return;
    }

    QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Reset autoincrement"), tr("Are you sure you want to reset autoincrement value for table '%1'?")
                                                            .arg(table));
    if (btn != QMessageBox::Yes)
        return;

    SqlQueryPtr res = db->exec("DELETE FROM sqlite_sequence WHERE name = ?;", {table});
    if (res->isError())
        notifyError(tr("An error occurred while trying to reset autoincrement value for table '%1': %2").arg(table, res->getErrorText()));
    else
        notifyInfo(tr("Autoincrement value for table '%1' has been reset successfully.").arg(table));
}

void DbTree::eraseTableData()
{
    Db* db = getSelectedDb();
    if (!db || !db->isValid())
        return;

    QList<DbTreeItem *> items = getSelectedItems(DbTreeItem::Type::TABLE);
    if (items.isEmpty())
    {
        qWarning() << "Tried to erase table data, while table wasn't selected in DbTree.";
        return;
    }

    QStringList tables;
    for (DbTreeItem* item : items)
        tables << item->getTable();

    QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Erase table data"), tr("Are you sure you want to delete all data from table(s): %1?")
                                                            .arg(tables.join(", ")));
    if (btn != QMessageBox::Yes)
        return;

    static_qstring(DELETE_SQL, "DELETE FROM %1;");
    SqlQueryPtr res;
    for (const QString& table : tables)
    {
        res = db->exec(DELETE_SQL.arg(wrapObjIfNeeded(table)));
        if (res->isError())
        {
            notifyError(tr("An error occurred while trying to delete data from table '%1': %2").arg(table, res->getErrorText()));
            return;
        }

        notifyInfo(tr("All data has been deleted for table '%1'.").arg(table));
    }
}

void DbTree::addColumn(DbTreeItem* item)
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    DbTreeItem* tableItem = nullptr;

    if (item->getType() == DbTreeItem::Type::TABLE)
        tableItem = item;
    else
        tableItem = item->findParentItem(DbTreeItem::Type::TABLE);

    if (!tableItem)
        return;

    TableWindow* tableWin = openTable(tableItem);
    tableWin->addColumn();
}

void DbTree::editColumn(DbTreeItem* item)
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    if (item->getType() != DbTreeItem::Type::COLUMN)
        return;

    DbTreeItem* tableItem = item->findParentItem(DbTreeItem::Type::TABLE);
    if (!tableItem)
        return;

    TableWindow* tableWin = openTable(tableItem);
    tableWin->editColumn(item->text());
}

void DbTree::delColumn(DbTreeItem* item)
{
    Db* db = getSelectedOpenDb();
    if (!db || !db->isValid())
        return;

    if (item->getType() != DbTreeItem::Type::COLUMN)
        return;

    DbTreeItem* tableItem = item->findParentItem(DbTreeItem::Type::TABLE);
    if (!tableItem)
        return;

    TableWindow* tableWin = openTable(tableItem);
    tableWin->delColumn(item->text());
}

void DbTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    UNUSED(previous);
    updateActionStates(treeModel->itemFromIndex(current));
}

void DbTree::deleteSelected(ItemFilterFunc filterFunc)
{
    deleteItems(getSelectedItems(filterFunc));
}

QList<DbTreeItem*> DbTree::getSelectedItems(DbTree::ItemFilterFunc filterFunc)
{
    QModelIndexList idxList = ui->treeView->getSelectedIndexes();
    QList<DbTreeItem*> items;
    DbTreeItem* item;
    for (const QModelIndex& idx : idxList)
    {
        item = dynamic_cast<DbTreeItem*>(treeModel->itemFromIndex(idx));
        if (filterFunc && !filterFunc(item))
            continue;

        items << item;
    }
    return items;
}

void DbTree::changeFontSize(int factor)
{
    auto f = CFG_UI.Fonts.DbTree.get();
    f.setPointSize(f.pointSize() + factor);
    CFG_UI.Fonts.DbTree.set(f);

    f = CFG_UI.Fonts.DbTreeLabel.get();
    f.setPointSize(f.pointSize() + factor);
    CFG_UI.Fonts.DbTreeLabel.set(f);
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
    QString itemStr;
    int groupItems = 0;
    for (DbTreeItem* item : items)
    {
        itemStr = itemTmp.arg(item->getIcon()->toUrl()).arg(item->text().left(ITEM_TEXT_LIMIT));

        if (item->getType() == DbTreeItem::Type::DB)
            databasesToRemove << itemStr;
        else
            toDelete << itemStr;

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
    QSet<Db*> deletedDatabases;
    QSet<Db*> databasesToRefresh;
    for (DbTreeItem* item : items)
    {
        if (item->getType() == DbTreeItem::Type::DB)
            deletedDatabases << item->getDb();

        databasesToRefresh << item->getDb();
        deleteItem(item);
    }

    for (Db* dbToRefresh : databasesToRefresh)
    {
        if (deletedDatabases.contains(dbToRefresh))
            continue;

        refreshSchema(dbToRefresh);
    }

    emit sessionValueChanged();
}

void DbTree::refreshSchemas()
{
    for (Db* db : DBLIST->getDbList())
        treeModel->refreshSchema(db);

    updateActionsForCurrent();
}

void DbTree::interrupt()
{
    treeModel->interrupt();
}

void DbTree::refreshSchema()
{
    Db* db = getSelectedDb();
    refreshSchema(db);
}

void DbTree::updateActionsForCurrent()
{
    updateActionStates(ui->treeView->currentItem());
}

void DbTree::setFileExecProgress(int newValue)
{
    fileExecWidgetCover->setProgress(newValue);
}

void DbTree::hideFileExecCover()
{
    fileExecWidgetCover->hide();
}

void DbTree::showFileExecErrors(const QList<QPair<QString, QString> >& errors, bool rolledBack)
{
    FileExecErrorsDialog dialog(errors, rolledBack, MAINWINDOW);
    dialog.exec();
}

void DbTree::fontSizeChangeRequested(int delta)
{
    changeFontSize(delta >= 0 ? 1 : -1);
}

void DbTree::incrFontSize()
{
    changeFontSize(1);
}

void DbTree::decrFontSize()
{
    changeFontSize(-1);
}

void DbTree::dbConnected(Db* db)
{
    updateActionsForCurrent();
    updateDbIcon(db);
    emit sessionValueChanged();
}

void DbTree::dbDisconnected(Db* db)
{
    updateActionsForCurrent();
    updateDbIcon(db);
    emit sessionValueChanged();
}

void DbTree::updateDbIcon(Db* db)
{
    DbTreeItem* item = treeModel->findItem(DbTreeItem::Type::DB, db);
    if (item)
        item->updateDbIcon();
}

void DbTree::refreshFont()
{
    ui->treeView->doItemsLayout();
}

void DbTree::generateSelectForTable()
{
    Db* db = getSelectedDb();
    QString table = getSelectedTableName();

    QueryGenerator generator;
    QString sql = generator.generateSelectFromTable(db, table);
    MAINWINDOW->openSqlEditor(db, sql);
}

void DbTree::generateInsertForTable()
{
    Db* db = getSelectedDb();
    QString table = getSelectedTableName();

    QueryGenerator generator;
    QString sql = generator.generateInsertToTable(db, table);
    MAINWINDOW->openSqlEditor(db, sql);
}

void DbTree::generateUpdateForTable()
{
    Db* db = getSelectedDb();
    QString table = getSelectedTableName();

    QueryGenerator generator;
    QString sql = generator.generateUpdateOfTable(db, table);
    MAINWINDOW->openSqlEditor(db, sql);
}

void DbTree::generateDeleteForTable()
{
    Db* db = getSelectedDb();
    QString table = getSelectedTableName();

    QueryGenerator generator;
    QString sql = generator.generateDeleteFromTable(db, table);
    MAINWINDOW->openSqlEditor(db, sql);
}

void DbTree::openDbDirectory()
{
    Db* db = getSelectedDb();
    if (!db)
        return;

    QFileInfo fi(db->getPath());
    if (!fi.exists())
        return;

    QUrl url = QUrl::fromLocalFile(fi.dir().path());
    if (url.isValid())
        QDesktopServices::openUrl(url);
}

void DbTree::execSqlFromFile()
{
    Db* db = getSelectedDb();
    if (!db || !db->isOpen())
        return;

    ExecFromFileDialog dialog(MAINWINDOW);
    int res = dialog.exec();
    if (res != QDialog::Accepted)
        return;

    if (fileExecutor->isExecuting())
        return;

    fileExecWidgetCover->show();
    fileExecutor->execSqlFromFile(db, dialog.filePath(), dialog.ignoreErrors(), dialog.codec());
}

void DbTree::setupDefShortcuts()
{
    setShortcutContext({
                           CLEAR_FILTER, DEL_SELECTED, REFRESH_SCHEMA, REFRESH_SCHEMAS,
                           ADD_DB, SELECT_ALL, COPY, PASTE
                       }, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(DbTree, Action);
}

int qHash(DbTree::Action action)
{
    return static_cast<int>(action);
}
