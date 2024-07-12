#include "dbtreemodel.h"
#include "services/dbmanager.h"
#include "dbtreeview.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include "schemaresolver.h"
#include "dbtreeitemfactory.h"
#include "common/unused.h"
#include "services/pluginmanager.h"
#include "plugins/dbplugin.h"
#include "dbobjectorganizer.h"
#include "dialogs/dbdialog.h"
#include "dialogs/errorsconfirmdialog.h"
#include "dialogs/versionconvertsummarydialog.h"
#include "db/invaliddb.h"
#include "services/notifymanager.h"
#include "common/compatibility.h"
#include <QMimeData>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QCheckBox>
#include <QWidgetAction>
#include <QClipboard>

const QString DbTreeModel::toolTipTableTmp = "<table>%1</table>";
const QString DbTreeModel::toolTipHdrRowTmp = "<tr><th><img src=\"%1\"/></th><th colspan=2>%2</th></tr>";
const QString DbTreeModel::toolTipRowTmp = "<tr><td></td><td>%1</td><td align=\"right\">%2</td></tr>";
const QString DbTreeModel::toolTipIconRowTmp = "<tr><td><img src=\"%1\"/></td><td>%2</td><td align=\"right\">%3</td></tr>";

DbTreeModel::DbTreeModel()
{
    setItemPrototype(DbTreeItemFactory::createPrototype());
    connectDbManagerSignals();

    connect(CFG, SIGNAL(massSaveBegins()), this, SLOT(massSaveBegins()));
    connect(CFG, SIGNAL(massSaveCommitted()), this, SLOT(massSaveCommitted()));
    connect(CFG_UI.General.ShowSystemObjects, SIGNAL(changed(QVariant)), this, SLOT(markSchemaReloadingRequired()));

    dbOrganizer = new DbObjectOrganizer(confirmReferencedTables, resolveNameConflict, confirmConversion, confirmConversionErrors);
    dbOrganizer->setAutoDelete(false);
    connect(dbOrganizer, SIGNAL(finishedDbObjectsCopy(bool,Db*,Db*)), this, SLOT(dbObjectsCopyFinished(bool,Db*,Db*)));
    connect(dbOrganizer, SIGNAL(finishedDbObjectsMove(bool,Db*,Db*)), this, SLOT(dbObjectsMoveFinished(bool,Db*,Db*)));
}

DbTreeModel::~DbTreeModel()
{
}

void DbTreeModel::connectDbManagerSignals()
{
    connect(DBLIST, SIGNAL(dbAdded(Db*)), this, SLOT(dbAdded(Db*)));
    connect(DBLIST, SIGNAL(dbUpdated(QString,Db*)), this, SLOT(dbUpdated(QString,Db*)));
    connect(DBLIST, SIGNAL(dbRemoved(Db*)), this, SLOT(dbRemoved(Db*)));
    connect(DBLIST, SIGNAL(dbConnected(Db*)), this, SLOT(dbConnected(Db*)));
    connect(DBLIST, SIGNAL(dbDisconnected(Db*)), this, SLOT(dbDisconnected(Db*)));
    connect(DBLIST, SIGNAL(dbLoaded(Db*)), this, SLOT(dbLoaded(Db*)));
    connect(DBLIST, SIGNAL(dbUnloaded(Db*)), this, SLOT(dbUnloaded(Db*)));
}

void DbTreeModel::move(QStandardItem *itemToMove, QStandardItem *newParentItem, int newRow)
{
    QStandardItem* currParent = dynamic_cast<DbTreeItem*>(itemToMove)->parentItem();
    if (!newParentItem)
        newParentItem = root();

    if (newParentItem == currParent)
    {
        move(itemToMove, newRow);
        return;
    }

    int oldRow = itemToMove->index().row();
    currParent->takeRow(oldRow);

    if (newRow > currParent->rowCount() || newRow < 0)
        newParentItem->appendRow(itemToMove);
    else
        newParentItem->insertRow(newRow, itemToMove);
}

void DbTreeModel::move(QStandardItem *itemToMove, int newRow)
{
    QStandardItem* currParent = dynamic_cast<DbTreeItem*>(itemToMove)->parentItem();
    int oldRow = itemToMove->index().row();
    currParent->takeRow(oldRow);
    if (newRow > currParent->rowCount() || newRow  < 0)
        currParent->appendRow(itemToMove);
    else if (oldRow < newRow)
        currParent->insertRow(newRow - 1, itemToMove);
    else
        currParent->insertRow(newRow, itemToMove);
}

void DbTreeModel::deleteGroup(QStandardItem *groupItem)
{
    QStandardItem* parentItem = dynamic_cast<DbTreeItem*>(groupItem)->parentItem();
    if (!parentItem)
        parentItem = root();

    for (QStandardItem* child : dynamic_cast<DbTreeItem*>(groupItem)->childs())
        move(child, parentItem);

    parentItem->removeRow(groupItem->row());
}

DbTreeItem* DbTreeModel::createGroup(const QString& name, QStandardItem* parent)
{
    if (!parent)
        parent = root();

    DbTreeItem* item = DbTreeItemFactory::createDir(name, this);
    parent->appendRow(item);
    return item;
}

QStringList DbTreeModel::getGroupFor(QStandardItem *item)
{
    QStringList group;
    while ((item = item->parent()) != nullptr)
    {
        if (dynamic_cast<DbTreeItem*>(item)->getType() == DbTreeItem::Type::DIR)
            group.prepend(item->text());
    }
    return group;
}

void DbTreeModel::applyFilter(const QString &filter)
{
    applyFilter(root(), filter);
    currentFilter = filter;
}

bool DbTreeModel::applyFilter(QStandardItem *parentItem, const QString &filter)
{
    bool empty = filter.isEmpty();
    bool visibilityForParent = false;
    DbTreeItem* item = nullptr;
    QModelIndex index;
    bool subFilterResult;
    bool matched;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
         item = dynamic_cast<DbTreeItem*>(parentItem->child(i));
         index = item->index();
         subFilterResult = applyFilter(item, filter);
         matched = empty || subFilterResult || item->text().contains(filter, Qt::CaseInsensitive);
         treeView->setRowHidden(index.row(), index.parent(), !matched);

         if (matched)
             visibilityForParent = true;
    }
    return visibilityForParent;
}

void DbTreeModel::storeGroups()
{
    QList<Config::DbGroupPtr> groups = childsToConfig(invisibleRootItem());
    CFG->storeGroups(groups);
}

void DbTreeModel::readGroups(QList<Db*> dbList)
{
    QList<Config::DbGroupPtr> groups = CFG->getGroups();
    for (const Config::DbGroupPtr& group : groups)
        restoreGroup(group, &dbList);

    // Add rest of databases, not mentioned in groups
    Config::DbGroupPtr group;
    for (Db* db : dbList)
    {
        group = Config::DbGroupPtr::create();
        group->referencedDbName = db->getName();
        restoreGroup(group);
    }
}

QList<Config::DbGroupPtr> DbTreeModel::childsToConfig(QStandardItem *item)
{
    QList<Config::DbGroupPtr> groups;
    Config::DbGroupPtr group;
    DbTreeItem* dbTreeItem = nullptr;
    for (int i = 0; i < item->rowCount(); i++)
    {
        dbTreeItem = dynamic_cast<DbTreeItem*>(item->child(i));
        switch (dbTreeItem->getType())
        {
            case DbTreeItem::Type::DIR:
            {
                group = Config::DbGroupPtr::create();
                group->name = dbTreeItem->text();
                group->order = i;
                group->open = treeView->isExpanded(dbTreeItem->index());
                group->childs = childsToConfig(dbTreeItem);
                groups += group;
                break;
            }
            case DbTreeItem::Type::DB:
            {
                group = Config::DbGroupPtr::create();
                group->referencedDbName = dbTreeItem->text();
                group->order = i;
                group->open = dbTreeItem->getDb()->isOpen();
                group->dbExpanded = treeView->isExpanded(dbTreeItem->index());
                groups += group;
                break;
            }
            default:
                // no-op
                break;
        }
    }
    return groups;
}

void DbTreeModel::restoreGroup(const Config::DbGroupPtr& group, QList<Db*>* dbList, QStandardItem* parent)
{
    Db* db = nullptr;
    DbTreeItem* item = nullptr;
    if (group->referencedDbName.isNull())
    {
        item = DbTreeItemFactory::createDir(group->name, this);
    }
    else
    {
        db = DBLIST->getByName(group->referencedDbName);

        // Databases referenced in groups must exist on database list. If not, we ignore them.
        // Even invalid (no plugin, no file) databases have entry in dblist.
        if (!db)
            return;

        item = DbTreeItemFactory::createDb(group->referencedDbName, this);
        item->setDb(group->referencedDbName);
        if (dbList)
            dbList->removeOne(db);
    }

    if (!parent)
        parent = invisibleRootItem();

    parent->appendRow(item);

    if (item->getType() == DbTreeItem::Type::DIR)
    {
        for (const Config::DbGroupPtr& childGroup : group->childs)
            restoreGroup(childGroup, dbList, item);
    }

    if (group->open)
    {
        if (db)
        {
            // If the db was stored in cfg as open, it was already open by DbManager.
            // Now the DbTreeModel didn't catch that (as it didn't exist yet), so we need to
            // call handler for 'connected' event, instead of forcing another open call.
            // Otherwise the database that could not be open would be requested to open twice:
            // 1. when restoring DbManager
            // 2. here
            // Instead of that, we just check if the database is already open (by DbManager)
            // and call proper handler to refresh database's schema and create tree nodes.
            if (db->isOpen())
                dbConnected(db, group->dbExpanded);
        }
        else
        {
            treeView->expand(item->index());
        }
    }
}

void DbTreeModel::expanded(const QModelIndex &index)
{
    QStandardItem* item = itemFromIndex(index);
    if (!item->hasChildren())
    {
        treeView->collapse(index);
        return;
    }

    if (dynamic_cast<DbTreeItem*>(item)->getType() == DbTreeItem::Type::DIR)
        itemFromIndex(index)->setIcon(ICONS.DIRECTORY_OPEN);
}

void DbTreeModel::collapsed(const QModelIndex &index)
{
    QStandardItem* item = itemFromIndex(index);
    if (dynamic_cast<DbTreeItem*>(item)->getType() == DbTreeItem::Type::DIR)
        item->setIcon(ICONS.DIRECTORY_OPEN);
}

void DbTreeModel::dbAdded(Db* db)
{
    DbTreeItem* item = DbTreeItemFactory::createDb(db->getName(), this);
    item->setDb(db);
    root()->appendRow(item);
}

void DbTreeModel::dbUpdated(const QString& oldName, Db* db)
{
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(findItem(DbTreeItem::Type::DB, oldName));
    if (!item)
    {
        qWarning() << "Updated database in db model that couldn't be found in the model:" << oldName;
        return;
    }

    item->setText(db->getName());
    item->setDb(db->getName());
    item->updateDbIcon();
}

void DbTreeModel::dbRemoved(Db* db)
{
    dbRemoved(db->getName());
}

void DbTreeModel::dbRemoved(const QString& name)
{
    QStandardItem* item = findItem(DbTreeItem::Type::DB, name);
    if (!item)
    {
        qWarning() << "Removed database from db model that couldn't be found in the model:" << name;
        return;
    }
    dbRemoved(item);
}

void DbTreeModel::dbRemoved(QStandardItem* item)
{
    QStandardItem* parent = item->parent();
    if (!parent)
        parent = root();

    parent->removeRow(item->index().row());
    if (!parent->hasChildren())
        treeView->collapse(parent->index());
}

void DbTreeModel::interrupt()
{
    dbOrganizer->interrupt();
}

void DbTreeModel::refreshSchema(Db* db)
{
    QStandardItem* item = findItem(DbTreeItem::Type::DB, db);
    if (!item)
    {
        qWarning() << "Refreshing schema of db that couldn't be found in the model:" << db->getName();
        return;
    }
    refreshSchema(db, item);
    applyFilter(item, currentFilter);
}

QList<DbTreeItem*> DbTreeModel::getAllItemsAsFlatList() const
{
    return getChildsAsFlatList(root());
}

QList<DbTreeItem*> DbTreeModel::getChildsAsFlatList(QStandardItem* item) const
{
    QList<DbTreeItem*> items;
    QStandardItem* child = nullptr;
    for (int i = 0; i < item->rowCount(); i++)
    {
        child = item->child(i);
        items << dynamic_cast<DbTreeItem*>(child);
        items += getChildsAsFlatList(child);
    }
    return items;
}

QVariant DbTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QStandardItemModel::data(index, role);;

    DbTreeItem* item = dynamic_cast<DbTreeItem*>(itemFromIndex(index));
    switch (role)
    {
        case Qt::ToolTipRole:
        {
            return getToolTip(item);
        }
    }
    return QStandardItemModel::data(index, role);
}

QString DbTreeModel::getToolTip(DbTreeItem* item) const
{
    if (!item)
        return QString();

    switch (item->getType())
    {
        case DbTreeItem::Type::DB:
            return getDbToolTip(item);
        case DbTreeItem::Type::TABLE:
            return getTableToolTip(item);
        default:
            break;
    }
    return QString();
}

QString DbTreeModel::getDbToolTip(DbTreeItem* item) const
{
    QStringList rows;

    Db* db = item->getDb();
    QString iconPath = db->isValid() ? ICONS.DATABASE.toImgSrc() : ICONS.DATABASE_INVALID.toImgSrc();
    int fileSize = -1;

    QUrl url(db->getPath());
    if (url.scheme().isEmpty() || url.scheme() == "file")
        fileSize = QFile(db->getPath()).size();

    rows << toolTipHdrRowTmp.arg(iconPath).arg(tr("Database: %1", "dbtree tooltip").arg(db->getName()));
    rows << toolTipRowTmp.arg(tr("URI:", "dbtree tooltip")).arg(toNativePath(db->getPath()));

    if (db->isValid())
    {
        rows << toolTipRowTmp.arg(tr("Version:", "dbtree tooltip")).arg(QString("SQLite %1").arg(db->getVersion()));

        if (fileSize > -1)
            rows << toolTipRowTmp.arg(tr("File size:", "dbtree tooltip")).arg(formatFileSize(fileSize));

        if (db->isOpen())
            rows << toolTipRowTmp.arg(tr("Encoding:", "dbtree tooltip")).arg(db->getEncoding());
    }
    else
    {
        InvalidDb* idb = dynamic_cast<InvalidDb*>(db);
        rows << toolTipRowTmp.arg(tr("Error:", "dbtree tooltip")).arg(idb->getError());
    }

    return toolTipTableTmp.arg(rows.join(""));
}

QString DbTreeModel::getTableToolTip(DbTreeItem* item) const
{
    QStringList rows;

    rows << toolTipHdrRowTmp.arg(ICONS.TABLE.getPath()).arg(tr("Table : %1", "dbtree tooltip").arg(item->text()));

    QStandardItem* columnsItem = item->child(0);
    QStandardItem* indexesItem = item->child(1);
    QStandardItem* triggersItem = item->child(2);

    int columnCnt = columnsItem->rowCount();
    int indexesCount = indexesItem->rowCount();
    int triggersCount = triggersItem->rowCount();

    QStringList columns;
    for (int i = 0; i < columnCnt; i++)
        columns << columnsItem->child(i)->text();

    QStringList indexes;
    for (int i = 0; i < indexesCount; i++)
        indexes << indexesItem->child(i)->text();

    QStringList triggers;
    for (int i = 0; i < triggersCount; i++)
        triggers << triggersItem->child(i)->text();

    rows << toolTipIconRowTmp.arg(ICONS.COLUMN.getPath())
                             .arg(tr("Columns (%1):", "dbtree tooltip").arg(columnCnt))
                             .arg(columns.join(", "));
    rows << toolTipIconRowTmp.arg(ICONS.INDEX.getPath())
                             .arg(tr("Indexes (%1):", "dbtree tooltip").arg(indexesCount))
                             .arg(indexes.join(", "));
    rows << toolTipIconRowTmp.arg(ICONS.TRIGGER.getPath())
                             .arg(tr("Triggers (%1):", "dbtree tooltip").arg(triggersCount))
                             .arg(triggers.join(", "));

    return toolTipTableTmp.arg(rows.join(""));
}

void DbTreeModel::refreshSchema(Db* db, QStandardItem *item)
{
    if (!db->isOpen())
        return;

    // Remember expanded state of this branch
    QHash<QString, bool> expandedState;
    collectExpandedState(expandedState, item);

    // Delete child nodes
    while (item->rowCount() > 0)
        item->removeRow(0);

    // Now prepare to create new branch
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(!CFG_UI.General.ShowSystemObjects.get());

    // Collect all db objects and build the db branch
    bool sort = CFG_UI.General.SortObjects.get();
    QStringList tables = resolver.getTables();
    QStringList virtualTables;
    for (const QString& table : tables)
    {
        if (resolver.isVirtualTable(table))
            virtualTables << table;
    }

    QList<QStandardItem*> tableItems = refreshSchemaTables(tables, virtualTables, sort);
    StrHash<QList<QStandardItem*>> allTableColumns = refreshSchemaTableColumns(resolver.getAllTableColumns());
    StrHash<QList<QStandardItem*>> indexItems = refreshSchemaIndexes(resolver.getGroupedIndexes(), sort);
    StrHash<QList<QStandardItem*>> triggerItems = refreshSchemaTriggers(resolver.getGroupedTriggers(), sort);
    QList<QStandardItem*> viewItems = refreshSchemaViews(resolver.getViews(), sort);
    refreshSchemaBuild(item, tableItems, indexItems, triggerItems, viewItems, allTableColumns);
    populateChildItemsWithDb(item, db);
    restoreExpandedState(expandedState, item);
}

void DbTreeModel::collectExpandedState(QHash<QString, bool> &state, QStandardItem *parentItem)
{
    if (!parentItem)
        parentItem = root();

    DbTreeItem* dbTreeItem = dynamic_cast<DbTreeItem*>(parentItem);
    if (dbTreeItem)
        state[dbTreeItem->signature()] = treeView->isExpanded(dbTreeItem->index());

    for (int i = 0; i < parentItem->rowCount(); i++)
        collectExpandedState(state, parentItem->child(i));
}

QList<QStandardItem *> DbTreeModel::refreshSchemaTables(const QStringList &tables, const QStringList& virtualTables, bool sort)
{
    QStringList sortedTables = tables;
    if (sort)
        sortedTables.sort(Qt::CaseInsensitive);

    QList<QStandardItem *> items;
    for (const QString& table : sortedTables)
    {
        if (virtualTables.contains(table))
            items += DbTreeItemFactory::createVirtualTable(table, this);
        else
            items += DbTreeItemFactory::createTable(table, this);
    }

    return items;
}

StrHash<QList<QStandardItem*>> DbTreeModel::refreshSchemaTableColumns(const StrHash<QStringList> &columns)
{
    QStringList sortedColumns;
    bool doSort = CFG_UI.General.SortColumns.get();
    StrHash<QList<QStandardItem*>> items;
    for (const QString& key : columns.keys())
    {
        sortedColumns = columns[key];
        if (doSort)
            ::sSort(sortedColumns);

        for (const QString& column : sortedColumns)
            items[key] += DbTreeItemFactory::createColumn(column, this);
    }
    return items;
}

StrHash<QList<QStandardItem *> > DbTreeModel::refreshSchemaIndexes(const StrHash<QStringList> &indexes, bool sort)
{
    StrHash<QList<QStandardItem *> > items;
    QStringList sortedIndexes;
    for (const QString& key : indexes.keys())
    {
        sortedIndexes = indexes[key];
        if (sort)
            sortedIndexes.sort(Qt::CaseInsensitive);

        for (const QString& index : sortedIndexes)
            items[key] += DbTreeItemFactory::createIndex(index, this);
    }
    return items;
}

StrHash<QList<QStandardItem*>> DbTreeModel::refreshSchemaTriggers(const StrHash<QStringList> &triggers, bool sort)
{
    StrHash<QList<QStandardItem*>> items;
    QStringList sortedTriggers;
    for (const QString& key : triggers.keys())
    {
        sortedTriggers = triggers[key];
        if (sort)
            sortedTriggers.sort(Qt::CaseInsensitive);

        for (const QString& trigger : sortedTriggers)
            items[key] += DbTreeItemFactory::createTrigger(trigger, this);
    }
    return items;
}

QList<QStandardItem *> DbTreeModel::refreshSchemaViews(const QStringList &views, bool sort)
{
    QStringList sortedViews = views;
    if (sort)
        sortedViews.sort(Qt::CaseInsensitive);

    QList<QStandardItem *> items;
    for (const QString& view : sortedViews)
        items += DbTreeItemFactory::createView(view, this);

    return items;
}

void DbTreeModel::populateChildItemsWithDb(QStandardItem *parentItem, Db* db)
{
    QStandardItem* childItem = nullptr;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
        childItem = parentItem->child(i);
        dynamic_cast<DbTreeItem*>(childItem)->setDb(db);
        populateChildItemsWithDb(childItem, db);
    }
}

void DbTreeModel::refreshSchemaBuild(QStandardItem *dbItem,
                                     QList<QStandardItem*> tables,
                                     StrHash<QList<QStandardItem*> > indexes,
                                     StrHash<QList<QStandardItem*> > triggers,
                                     QList<QStandardItem*> views,
                                     StrHash<QList<QStandardItem*> > allTableColumns)
{
    DbTreeItem* tablesItem = DbTreeItemFactory::createTables(this);
    DbTreeItem* viewsItem = DbTreeItemFactory::createViews(this);

    dbItem->appendRow(tablesItem);
    dbItem->appendRow(viewsItem);

    DbTreeItem* columnsItem = nullptr;
    DbTreeItem* indexesItem = nullptr;
    DbTreeItem* triggersItem = nullptr;
    for (QStandardItem* tableItem : tables)
    {
        tablesItem->appendRow(tableItem);

        columnsItem = DbTreeItemFactory::createColumns(this);
        indexesItem = DbTreeItemFactory::createIndexes(this);
        triggersItem = DbTreeItemFactory::createTriggers(this);

        tableItem->appendRow(columnsItem);
        tableItem->appendRow(indexesItem);
        tableItem->appendRow(triggersItem);

        for (QStandardItem* columnItem : allTableColumns[tableItem->text()])
            columnsItem->appendRow(columnItem);

        for (QStandardItem* indexItem : indexes[tableItem->text()])
            indexesItem->appendRow(indexItem);

        for (QStandardItem* triggerItem : triggers[tableItem->text()])
            triggersItem->appendRow(triggerItem);
    }
    for (QStandardItem* viewItem : views)
    {
        viewsItem->appendRow(viewItem);

        triggersItem = DbTreeItemFactory::createTriggers(this);
        viewItem->appendRow(triggersItem);
        for (QStandardItem* triggerItem : triggers[viewItem->text()])
            triggersItem->appendRow(triggerItem);
    }
}

void DbTreeModel::restoreExpandedState(const QHash<QString, bool>& expandedState, QStandardItem* parentItem)
{
    DbTreeItem* parentDbTreeItem = dynamic_cast<DbTreeItem*>(parentItem);
    QString sig = parentDbTreeItem->signature();
    if (expandedState.contains(sig) && expandedState[sig])
        treeView->expand(parentItem->index());

    for (QStandardItem* child : parentDbTreeItem->childs())
        restoreExpandedState(expandedState, child);
}

DbTreeItem* DbTreeModel::findFirstItemOfType(DbTreeItem::Type type, QStandardItem* parentItem)
{
    DbTreeItem* child = nullptr;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
        child = dynamic_cast<DbTreeItem*>(parentItem->child(i));
        if (child->getType() == type)
            return child;

        child = findFirstItemOfType(type, child);
        if (child)
            return child;
    }
    return nullptr;
}

void DbTreeModel::dbConnected(Db* db, bool expandItem)
{
    QStandardItem* item = findItem(DbTreeItem::Type::DB, db);
    if (!item)
    {
        qWarning() << "Connected to db that couldn't be found in the model:" << db->getName();
        return;
    }
    refreshSchema(db, item);
    if (expandItem)
    {
        treeView->expand(item->index());
        if (CFG_UI.General.ExpandTables.get())
            treeView->expand(item->model()->index(0, 0, item->index())); // also expand tables

        if (CFG_UI.General.ExpandViews.get())
            treeView->expand(item->model()->index(1, 0, item->index())); // also expand views
    }
    treeView->setCurrentIndex(item->index());
}

void DbTreeModel::dbDisconnected(Db* db)
{
    QStandardItem* item = findItem(DbTreeItem::Type::DB, db);
    if (!item)
    {
        qWarning() << "Disconnected from db that couldn't be found in the model:" << db->getName();
        return;
    }

    while (item->rowCount() > 0)
        item->removeRow(0);

    treeView->collapse(item->index());
}

void DbTreeModel::dbUnloaded(Db* db)
{
    DbTreeItem* item = findItem(DbTreeItem::Type::DB, db->getName());
    if (!item)
    {
        qCritical() << "No DB item found to update icon:" << db->getName();
        return;
    }
    item->updateDbIcon();
}

void DbTreeModel::dbLoaded(Db* db)
{
    if (ignoreDbLoadedSignal)
        return;

    DbTreeItem* item = findItem(DbTreeItem::Type::DB, db->getName());
    if (!item)
    {
        qCritical() << "No DB item found to update icon:" << db->getName();
        return;
    }
    item->updateDbIcon();
}

void DbTreeModel::massSaveBegins()
{
    requireSchemaReloading = false;
}

void DbTreeModel::massSaveCommitted()
{
    if (requireSchemaReloading)
    {
        for (Db* db : DBLIST->getDbList())
        {
            if (db->isOpen())
                refreshSchema(db);
        }
    }
}

void DbTreeModel::markSchemaReloadingRequired()
{
    requireSchemaReloading = true;
}

DbTreeItem* DbTreeModel::findItem(DbTreeItem::Type type, const QString &name)
{
    return findItem(root(), type, name);
}

DbTreeItem *DbTreeModel::findItem(QStandardItem* parentItem, DbTreeItem::Type type, const QString& name)
{
    DbTreeItem* item = nullptr;
    DbTreeItem* subItem = nullptr;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
         item = dynamic_cast<DbTreeItem*>(parentItem->child(i));

         // Search recursively
         if (item->hasChildren())
         {
             subItem = findItem(item, type, name);
             if (subItem)
                 return subItem;
         }

         if (item->getType() != type)
             continue;

         if (item->text() != name)
             continue;

         return item;
    }

    return nullptr;
}

DbTreeItem *DbTreeModel::findItem(DbTreeItem::Type type, Db* db)
{
    return findItem(root(), type, db);
}

DbTreeItem* DbTreeModel::findFirstItemOfType(DbTreeItem::Type type)
{
    return findFirstItemOfType(type, root());
}

DbTreeItem *DbTreeModel::findItemBySignature(const QString &signature)
{
    QStringList parts = signature.split("_");
    QStringList pair;
    DbTreeItem* currItem = nullptr;
    DbTreeItem::Type type;
    QString name;
    for (const QString& part : parts)
    {
        pair = part.split(".");
        type = static_cast<DbTreeItem::Type>(pair.first().toInt());
        name = QString::fromUtf8(QByteArray::fromBase64(pair.last().toLatin1()));
        currItem = findItem((currItem ? currItem : root()), type, name);
        if (!currItem)
            return nullptr; // not found the target item
    }
    return currItem;
}

QList<DbTreeItem*> DbTreeModel::findItems(DbTreeItem::Type type)
{
    return findItems(root(), type);
}

DbTreeItem *DbTreeModel::findItem(QStandardItem* parentItem, DbTreeItem::Type type, Db* db)
{
    DbTreeItem* item = nullptr;
    DbTreeItem* subItem = nullptr;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
        item = dynamic_cast<DbTreeItem*>(parentItem->child(i));

        // Search recursively
        if (item->hasChildren())
        {
            subItem = findItem(item, type, db);
            if (subItem)
                return subItem;
        }

        if (item->getType() != type)
            continue;

        if (item->text() != db->getName())
            continue;

        return item;
    }

    return nullptr;
}

QList<DbTreeItem*> DbTreeModel::findItems(QStandardItem* parentItem, DbTreeItem::Type type)
{
    QList<DbTreeItem*> items;
    DbTreeItem* item = nullptr;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
        item = dynamic_cast<DbTreeItem*>(parentItem->child(i));

        // Search recursively
        if (item->getType() == DbTreeItem::Type::DIR)
            items += findItems(item, type);

        if (item->getType() != type)
            continue;

        items += item;
    }

    return items;
}

QStandardItem* DbTreeModel::root() const
{
    return invisibleRootItem();
}

void DbTreeModel::loadDbList()
{
    clear();
    readGroups(DBLIST->getDbList());
}

void DbTreeModel::itemChangedVisibility(DbTreeItem* item)
{
    emit updateItemHidden(item);
}

void DbTreeModel::setTreeView(DbTreeView *value)
{
    treeView = value;
    connect(treeView, &QTreeView::expanded, this, &DbTreeModel::expanded);
    connect(treeView, &QTreeView::collapsed, this, &DbTreeModel::collapsed);
    connect(this, SIGNAL(updateItemHidden(DbTreeItem*)), treeView, SLOT(updateItemHidden(DbTreeItem*)));
}

QStringList DbTreeModel::mimeTypes() const
{
    QStringList types = QStandardItemModel::mimeTypes();
    types << MIMETYPE;
    return types;
}

QMimeData *DbTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if (!data)
        return nullptr;

    if (indexes.size() == 0)
        return nullptr;

    QByteArray output;
    QDataStream stream(&output, QIODevice::WriteOnly);

    QList<QUrl> urlList;
    QStringList textList;

    DbTreeItem* item = nullptr;
    stream << indexes.size();
    for (const QModelIndex& idx : indexes)
    {
        item = dynamic_cast<DbTreeItem*>(itemFromIndex(idx));
        stream << item->signature();

        textList << item->text();
        if (item->getType() == DbTreeItem::Type::DB)
            urlList << QUrl("file://"+item->getDb()->getPath());
    }
    data->setData(MIMETYPE, output);
    data->setText(textList.join("\n"));
    data->setUrls(urlList);

    return data;
}

bool DbTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    UNUSED(action);
    // The result means: do we want the old item to be removed from the tree?
    bool invokeStdAction = false;
    bool res = pasteData(data, row, column, parent, Qt::IgnoreAction, &invokeStdAction);
    if (!invokeStdAction)
        return res;

    return QStandardItemModel::dropMimeData(data, action, row, column, parent);
}

bool DbTreeModel::pasteData(const QMimeData* data, int row, int column, const QModelIndex& parent, Qt::DropAction defaultAction, bool* invokeStdAction)
{
    // The result means: do we want the old item to be removed from the tree?
    DbTreeItem* dstItem = nullptr;
    if (parent.isValid())
    {
        QModelIndex idx = index(row, column, parent);
        if (idx.isValid())
            dstItem = dynamic_cast<DbTreeItem*>(itemFromIndex(idx));
        else // drop on top of the parent
            dstItem = dynamic_cast<DbTreeItem*>(itemFromIndex(parent));
    }
    else
    {
        dstItem = dynamic_cast<DbTreeItem*>(item(row, column));
    }

    if (data->formats().contains(MIMETYPE))
        return dropDbTreeItem(getDragItems(data), dstItem, defaultAction, invokeStdAction);
    else if (data->hasUrls())
        return dropUrls(data->urls());
    else
        return false;
}

void DbTreeModel::interruptableStarted(Interruptable* obj)
{
    if (interruptables.size() == 0)
        treeView->getDbTree()->showRefreshWidgetCover();

    interruptables << obj;
}

void DbTreeModel::interruptableFinished(Interruptable* obj)
{
    interruptables.removeOne(obj);
    if (interruptables.size() == 0)
        treeView->getDbTree()->hideRefreshWidgetCover();
}

QList<DbTreeItem*> DbTreeModel::getDragItems(const QMimeData* data)
{
    QList<DbTreeItem*> items;
    QByteArray byteData = data->data(MIMETYPE);
    QDataStream stream(&byteData, QIODevice::ReadOnly);

    qint32 itemCount;
    stream >> itemCount;

    DbTreeItem* item = nullptr;
    QString signature;
    for (qint32 i = 0; i < itemCount; i++)
    {
        stream >> signature;
        item = findItemBySignature(signature);
        if (item)
            items << item;
    }

    return items;
}

QList<DbTreeItem*> DbTreeModel::getItemsForIndexes(const QModelIndexList& indexes) const
{
    QList<DbTreeItem*> items;
    for (const QModelIndex& idx : indexes)
    {
        if (idx.isValid())
            items << dynamic_cast<DbTreeItem*>(itemFromIndex(idx));
    }

    return items;
}

void DbTreeModel::staticInit()
{
}

bool DbTreeModel::dropDbTreeItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem, Qt::DropAction defaultAction, bool *invokeStdDropAction)
{
    // The result means: do we want the old item to be removed from the tree?
    if (srcItems.size() == 0)
        return false;

    DbTreeItem* srcItem = srcItems.first();
    switch (srcItem->getType())
    {
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::VIEW:
        {
            if (!dstItem)
                return false;

            if (srcItem->getDb() == dstItem->getDb() && invokeStdDropAction)
            {
                *invokeStdDropAction = true;
                return true;
            }

            return dropDbObjectItem(srcItems, dstItem, defaultAction);
        }
        case DbTreeItem::Type::DB:
        case DbTreeItem::Type::DIR:
        {
            if (invokeStdDropAction)
                *invokeStdDropAction = true;

            break;
        }
        case DbTreeItem::Type::COLUMN:
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::INDEX:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEWS:
        case DbTreeItem::Type::COLUMNS:
        case DbTreeItem::Type::VIRTUAL_TABLE:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }

    return false;
}

bool DbTreeModel::dropDbObjectItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem, Qt::DropAction defaultAction)
{
    bool copy = false;
    bool move = false;
    bool includeData = false;
    bool includeIndexes = false;
    bool includeTriggers = false;

    if (defaultAction == Qt::CopyAction)
    {
        copy = true;
        includeData = true;
        includeIndexes = true;
        includeTriggers = true;
    }
    else if (defaultAction == Qt::MoveAction)
    {
        move = true;
        includeData = true;
        includeIndexes = true;
        includeTriggers = true;
    }
    else
    {
        QMenu menu;
        QAction* copyAction = menu.addAction(ICONS.ACT_COPY, tr("Copy"));
        QAction* moveAction = menu.addAction(ICONS.ACT_CUT, tr("Move"));
        menu.addSeparator();
        QCheckBox *includeDataCheck = createCopyOrMoveMenuCheckBox(&menu, tr("Include data"));
        QCheckBox *includeIndexesCheck = createCopyOrMoveMenuCheckBox(&menu, tr("Include indexes"));
        QCheckBox *includeTriggersCheck = createCopyOrMoveMenuCheckBox(&menu, tr("Include triggers"));
        menu.addSeparator();
        menu.addAction(ICONS.ACT_ABORT, tr("Abort"));

        connect(moveAction, &QAction::triggered, [&move]() {move = true;});
        connect(copyAction, &QAction::triggered, [&copy]() {copy = true;});

        menu.exec(treeView->mapToGlobal(treeView->getLastDropPosition()));

        includeData = includeDataCheck->isChecked();
        includeIndexes = includeIndexesCheck->isChecked();
        includeTriggers = includeTriggersCheck->isChecked();
    }

    // The result means: do we want the old item to be removed from the tree?
    if (!copy && !move)
        return false;

    moveOrCopyDbObjects(srcItems, dstItem, move, includeData, includeIndexes, includeTriggers);
    return move;
}

QCheckBox* DbTreeModel::createCopyOrMoveMenuCheckBox(QMenu* menu, const QString& label)
{
    QWidget* parentWidget = new QWidget(menu);
    parentWidget->setLayout(new QVBoxLayout());
    QMargins margins = parentWidget->layout()->contentsMargins();
    parentWidget->layout()->setContentsMargins(margins.left(), 0, margins.right(), 0);

    QCheckBox *cb = new QCheckBox(label);
    cb->setChecked(true);
    parentWidget->layout()->addWidget(cb);

    QWidgetAction *action = new QWidgetAction(menu);
    action->setDefaultWidget(parentWidget);
    menu->addAction(action);
    return cb;
}

bool DbTreeModel::dropUrls(const QList<QUrl>& urls)
{
    QString filePath;
    bool autoTest = false;
    for (const QUrl& url : urls)
    {
        if (!url.isLocalFile())
        {
            qDebug() << url.toString() + "skipped, not a local file.";
            continue;
        }

        autoTest = false;
        filePath = url.toLocalFile();
        if (CFG_UI.General.BypassDbDialogWhenDropped.get())
        {
            if (quickAddDroppedDb(filePath))
            {
                continue;
            }
            else
            {
                notifyWarn(tr("Could not add dropped database file '%1' automatically. Manual setup is necessary.").arg(filePath));
                autoTest = true;
            }
        }

        DbDialog dialog(DbDialog::ADD, MAINWINDOW);
        dialog.setPath(filePath);
        dialog.setDoAutoTest(autoTest);
        dialog.exec();
    }
    return false;
}

bool DbTreeModel::quickAddDroppedDb(const QString& filePath)
{
    DbPlugin* plugin = DBLIST->getPluginForDbFile(filePath);
    if (!plugin)
        return false;

    QString name = DBLIST->generateUniqueDbName(plugin, filePath);
    QHash<QString,QVariant> opts;
    opts[DB_PLUGIN] = plugin->getName();
    return DBLIST->addDb(name, filePath, opts, !CFG_UI.General.NewDbNotPermanentByDefault.get());
}

void DbTreeModel::moveOrCopyDbObjects(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem, bool move, bool includeData, bool includeIndexes, bool includeTriggers)
{
    if (srcItems.size() == 0)
        return;

    DbTreeItem* srcItem = srcItems.first();
    Db* srcDb = srcItem->getDb();
    Db* dstDb = dstItem->getDb();

    QStringList srcNames;
    for (DbTreeItem* item : srcItems)
        srcNames << item->text();

    interruptableStarted(dbOrganizer);
    if (move)
        dbOrganizer->moveObjectsToDb(srcDb, srcNames, dstDb, includeData, includeIndexes, includeTriggers);
    else
        dbOrganizer->copyObjectsToDb(srcDb, srcNames, dstDb, includeData, includeIndexes, includeTriggers);
}

bool DbTreeModel::confirmReferencedTables(const QStringList& tables)
{
    QMessageBox::StandardButton result = QMessageBox::question(MAINWINDOW, tr("Referenced tables"),
                                                               tr("Do you want to include following referenced tables as well:\n%1").arg(tables.join(", ")));

    return result == QMessageBox::Yes;
}

bool DbTreeModel::resolveNameConflict(QString& nameInConflict)
{
    bool ok = false;
    QInputDialog tmpDialog; // just for a cancel button text
    QString result = QInputDialog::getText(MAINWINDOW, tr("Name conflict"),
        tr("Following object already exists in the target database.\nPlease enter new, unique name, or "
           "press '%1' to abort the operation:").arg(tmpDialog.cancelButtonText()),
        QLineEdit::Normal, nameInConflict, &ok);

    if (ok)
        nameInConflict = result;

    return ok;
}

bool DbTreeModel::confirmConversion(const QList<QPair<QString, QString> >& diffs)
{
    VersionConvertSummaryDialog dialog(MAINWINDOW);
    dialog.setWindowTitle(tr("SQL statements conversion"));
    dialog.setSides(diffs);
    return dialog.exec() == QDialog::Accepted;
}

bool DbTreeModel::confirmConversionErrors(const QHash<QString, QSet<QString> >& errors)
{
    ErrorsConfirmDialog dialog(MAINWINDOW);
    dialog.setTopLabel(tr("Following error occurred while converting SQL statements to the target SQLite version:"));
    dialog.setBottomLabel(tr("Would you like to ignore those errors and proceed?"));
    dialog.setErrors(errors);
    return dialog.exec() == QDialog::Accepted;
}

bool DbTreeModel::getIgnoreDbLoadedSignal() const
{
    return ignoreDbLoadedSignal;
}

void DbTreeModel::setIgnoreDbLoadedSignal(bool value)
{
    ignoreDbLoadedSignal = value;
}

bool DbTreeModel::hasDbTreeItem(const QMimeData *data)
{
    return data->formats().contains(MIMETYPE);
}

void DbTreeModel::dbObjectsMoveFinished(bool success, Db* srcDb, Db* dstDb)
{
    if (!success)
    {
        interruptableFinished(dbOrganizer);
        DBTREE->refreshSchema(srcDb);
        return;
    }

    DBTREE->refreshSchema(srcDb);
    DBTREE->refreshSchema(dstDb);
    interruptableFinished(dbOrganizer);
}

void DbTreeModel::dbObjectsCopyFinished(bool success, Db* srcDb, Db* dstDb)
{
    dbObjectsMoveFinished(success, srcDb, dstDb);
}
