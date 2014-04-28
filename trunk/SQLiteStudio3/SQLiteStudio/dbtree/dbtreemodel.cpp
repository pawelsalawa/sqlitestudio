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
#include <QMimeData>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QCheckBox>
#include <QWidgetAction>

const QString DbTreeModel::toolTipTableTmp = "<table>%1</table>";
const QString DbTreeModel::toolTipHdrRowTmp = "<tr><th><img src=\"%1\"/></th><th colspan=2>%2</th></tr>";
const QString DbTreeModel::toolTipRowTmp = "<tr><td></td><td>%1</td><td align=\"right\">%2</td></tr>";
const QString DbTreeModel::toolTipIconRowTmp = "<tr><td><img src=\"%1\"/></td><td>%2</td><td align=\"right\">%3</td></tr>";

DbTreeModel::DbTreeModel()
{
    setItemPrototype(DbTreeItemFactory::createPrototype());
    connectDbManagerSignals();

    connect(CFG, &Config::massSaveBegins, this, &DbTreeModel::massSaveBegins);
    connect(CFG, &Config::massSaveCommited, this, &DbTreeModel::massSaveCommited);
    connect(CFG_UI.General.ShowSystemObjects, SIGNAL(changed(QVariant)), this, SLOT(markSchemaReloadingRequired()));

    dbOrganizer = new DbObjectOrganizer(confirmReferencedTables, resolveNameConflict);
    dbOrganizer->setAutoDelete(false);
    connect(dbOrganizer, SIGNAL(finishedDbObjectsCopy(bool,Db*,Db*)), this, SLOT(dbObjectsCopyFinished(bool,Db*,Db*)));
    connect(dbOrganizer, SIGNAL(finishedDbObjectsMove(bool,Db*,Db*)), this, SLOT(dbObjectsMoveFinished(bool,Db*,Db*)));
}

DbTreeModel::~DbTreeModel()
{
}

void DbTreeModel::connectDbManagerSignals()
{
    connect(DBLIST, &DbManager::dbAdded, this, &DbTreeModel::dbAdded);
    connect(DBLIST, &DbManager::dbUpdated, this, &DbTreeModel::dbUpdated);
    connect(DBLIST, SIGNAL(dbRemoved(Db*)), this, SLOT(dbRemoved(Db*)));
    connect(DBLIST, &DbManager::dbConnected, this, &DbTreeModel::dbConnected);
    connect(DBLIST, &DbManager::dbDisconnected, this, &DbTreeModel::dbDisconnected);
    connect(DBLIST, SIGNAL(dbLoaded(Db*,DbPlugin*)), this, SLOT(dbLoaded(Db*,DbPlugin*)));
    connect(DBLIST, SIGNAL(dbAboutToBeUnloaded(Db*,DbPlugin*)), this, SLOT(dbToBeUnloaded(Db*,DbPlugin*)));
}

void DbTreeModel::move(QStandardItem *itemToMove, QStandardItem *newParentItem, int newRow)
{
    QStandardItem* currParent = dynamic_cast<DbTreeItem*>(itemToMove)->parentItem();
    currParent->takeRow(itemToMove->index().row());
    if (newRow < 0)
        newParentItem->appendRow(itemToMove);
    else
        newParentItem->insertRow(newRow, itemToMove);
}

void DbTreeModel::move(QStandardItem *itemToMove, int newRow)
{
    QStandardItem* currParent = dynamic_cast<DbTreeItem*>(itemToMove)->parentItem();
    currParent->takeRow(itemToMove->index().row());
    currParent->insertRow(newRow, itemToMove);
}

void DbTreeModel::deleteGroup(QStandardItem *groupItem)
{
    QStandardItem* parentItem = dynamic_cast<DbTreeItem*>(groupItem)->parentItem();
    if (!parentItem)
        parentItem = root();

    foreach (QStandardItem* child, dynamic_cast<DbTreeItem*>(groupItem)->childs())
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
}

bool DbTreeModel::applyFilter(QStandardItem *parentItem, const QString &filter)
{
    bool empty = filter.isEmpty();
    bool visibilityForParent = false;
    DbTreeItem* item;
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
    foreach (const Config::DbGroupPtr& group, groups)
        restoreGroup(group, &dbList);

    // Add rest of databases, not mentioned in groups
    Config::DbGroupPtr group;
    foreach (Db* db, dbList)
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
    DbTreeItem* dbTreeItem;
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
                groups += group;
                break;
            }
            case DbTreeItem::Type::INVALID_DB:
            {
                group = Config::DbGroupPtr::create();
                group->referencedDbName = dbTreeItem->text();
                group->order = i;
                group->open = false;
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
    DbTreeItem* item;
    bool invalidDb = false;
    if (group->referencedDbName.isNull())
    {
        item = DbTreeItemFactory::createDir(group->name, this);
    }
    else
    {
        // If db is managed by manager, it means it was successfully loaded.
        // Otherwise there was a problem with the file, or with plugin for that database
        // and we still want to have dbtree item for that database, we will just hide it.
        // Later, when plugin is loaded, item might become visible.
        item = DbTreeItemFactory::createDb(group->referencedDbName, this);
        item->setDb(group->referencedDbName);

        db = DBLIST->getByName(group->referencedDbName);
        if (!db)
            invalidDb = true;

        if (db && dbList)
            dbList->removeOne(db);
    }

    if (!parent)
        parent = invisibleRootItem();

    parent->appendRow(item);

    if (item->getType() == DbTreeItem::Type::DIR)
    {
        foreach (const Config::DbGroupPtr& childGroup, group->childs)
            restoreGroup(childGroup, dbList, item);
    }

    if (group->open)
    {
        if (db)
            db->open();

        treeView->expand(item->index());
    }

    if (invalidDb)
        item->setInvalidDbType(true);
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

void DbTreeModel::refreshSchema(Db* db)
{
    QStandardItem* item = findItem(DbTreeItem::Type::DB, db);
    if (!item)
    {
        qWarning() << "Refreshing schema of db that couldn't be found in the model:" << db->getName();
        return;
    }
    refreshSchema(db, item);
}

QList<DbTreeItem*> DbTreeModel::getAllItemsAsFlatList() const
{
    return getChildsAsFlatList(root());
}

QList<DbTreeItem*> DbTreeModel::getChildsAsFlatList(QStandardItem* item) const
{
    QList<DbTreeItem*> items;
    QStandardItem* child;
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
        return QString::null;

    switch (item->getType())
    {
        case DbTreeItem::Type::DB:
            return getDbToolTip(item);
        case DbTreeItem::Type::INVALID_DB:
            return getInvalidDbToolTip(item);
        case DbTreeItem::Type::TABLE:
            return getTableToolTip(item);
        default:
            break;
    }
    return QString::null;
}

QString DbTreeModel::getDbToolTip(DbTreeItem* item) const
{
    QStringList rows;

    Db* db = item->getDb();
    QFile dbFile(db->getPath());
    rows << toolTipHdrRowTmp.arg(ICONS.DATABASE.getPath()).arg(tr("Database: %1", "dbtree tooltip").arg(db->getName()));
    rows << toolTipRowTmp.arg("URI:").arg(db->getPath());
    rows << toolTipRowTmp.arg(tr("Version:", "dbtree tooltip")).arg(QString("SQLite %1").arg(db->getVersion()));
    rows << toolTipRowTmp.arg(tr("File size:", "dbtree tooltip")).arg(formatFileSize(dbFile.size()));
    rows << toolTipRowTmp.arg(tr("Encoding:", "dbtree tooltip")).arg(db->getEncoding());

    return toolTipTableTmp.arg(rows.join(""));
}

QString DbTreeModel::getInvalidDbToolTip(DbTreeItem* item) const
{
    QString dbName = item->text();
    Config::CfgDbPtr cfgDb = CFG->getDb(dbName);

    QStringList rows;
    //data:image/png;base64,
    rows << toolTipHdrRowTmp.arg(ICONS.DATABASE_INVALID.toBase64Url()).arg(tr("Database: %1", "dbtree tooltip").arg(dbName));

    QString errorMsg;
    if (cfgDb)
    {
        rows << toolTipRowTmp.arg("URI:").arg(cfgDb->path);

        QFileInfo file(cfgDb->path);
        if (file.isReadable())
            errorMsg = tr("Invalid SQLite database file, missing driver plugin, or encrypted database.");
        else
            errorMsg = tr("File not readable.");
    }
    else
    {
        errorMsg = tr("Database not found in configuration. Remove it and add it again.");
    }
    rows << toolTipRowTmp.arg("Error:").arg(errorMsg);
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

    // TODO all this below should be done in a separate thread
    // TODO also disable any operations on this item (db) during refreshing

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
    QHash<QString, QList<QStandardItem *> > allTableColumns = refreshSchemaTableColumns(resolver.getAllTableColumns());
    QMap<QString,QList<QStandardItem*> > indexItems = refreshSchemaIndexes(resolver.getGroupedIndexes(), sort);
    QMap<QString,QList<QStandardItem*> > triggerItems = refreshSchemaTriggers(resolver.getGroupedTriggers(), sort);
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
        qSort(sortedTables);

    QList<QStandardItem *> items;
    foreach (const QString& table, sortedTables)
    {
        if (virtualTables.contains(table))
            items += DbTreeItemFactory::createVirtualTable(table, this);
        else
            items += DbTreeItemFactory::createTable(table, this);
    }

    return items;
}

QHash<QString, QList<QStandardItem *> > DbTreeModel::refreshSchemaTableColumns(const QHash<QString, QStringList> &columns)
{
    QStringList sortedColumns;
    bool sort = CFG_UI.General.SortColumns.get();
    QHash<QString, QList<QStandardItem *> > items;
    QHashIterator<QString,QStringList> it(columns);
    while (it.hasNext())
    {
        it.next();
        sortedColumns = it.value();
        if (sort)
            qSort(sortedColumns);

        foreach (const QString& column, sortedColumns)
            items[it.key()] += DbTreeItemFactory::createColumn(column, this);
    }
    return items;
}

QMap<QString, QList<QStandardItem *> > DbTreeModel::refreshSchemaIndexes(const QMap<QString,QStringList> &indexes, bool sort)
{
    QMap<QString, QList<QStandardItem *> > items;
    QStringList sortedIndexes;
    QMapIterator<QString,QStringList> it(indexes);
    while (it.hasNext())
    {
        it.next();
        sortedIndexes = it.value();
        if (sort)
            qSort(sortedIndexes);

        foreach (const QString& index, sortedIndexes)
            items[it.key()] += DbTreeItemFactory::createIndex(index, this);
    }
    return items;
}

QMap<QString, QList<QStandardItem *> > DbTreeModel::refreshSchemaTriggers(const QMap<QString,QStringList> &triggers, bool sort)
{
    QMap<QString, QList<QStandardItem *> > items;
    QStringList sortedTriggers;
    QMapIterator<QString,QStringList> it(triggers);
    while (it.hasNext())
    {
        it.next();
        sortedTriggers = it.value();
        if (sort)
            qSort(sortedTriggers);

        foreach (const QString& trigger, sortedTriggers)
            items[it.key()] += DbTreeItemFactory::createTrigger(trigger, this);
    }
    return items;
}

QList<QStandardItem *> DbTreeModel::refreshSchemaViews(const QStringList &views, bool sort)
{
    QStringList sortedViews = views;
    if (sort)
        qSort(sortedViews);

    QList<QStandardItem *> items;
    foreach (const QString& view, views)
        items += DbTreeItemFactory::createView(view, this);

    return items;
}

void DbTreeModel::populateChildItemsWithDb(QStandardItem *parentItem, Db* db)
{
    QStandardItem* childItem;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
        childItem = parentItem->child(i);
        dynamic_cast<DbTreeItem*>(childItem)->setDb(db);
        populateChildItemsWithDb(childItem, db);
    }
}

void DbTreeModel::refreshSchemaBuild(QStandardItem *dbItem, QList<QStandardItem *> tables, QMap<QString,QList<QStandardItem*> > indexes,
                                     QMap<QString, QList<QStandardItem *> > triggers, QList<QStandardItem *> views, QHash<QString,
                                     QList<QStandardItem *> > allTableColumns)
{
    DbTreeItem* tablesItem = DbTreeItemFactory::createTables(this);
    DbTreeItem* viewsItem = DbTreeItemFactory::createViews(this);

    dbItem->appendRow(tablesItem);
    dbItem->appendRow(viewsItem);

    DbTreeItem* columnsItem;
    DbTreeItem* indexesItem;
    DbTreeItem* triggersItem;
    foreach (QStandardItem* tableItem, tables)
    {
        tablesItem->appendRow(tableItem);

        columnsItem = DbTreeItemFactory::createColumns(this);
        indexesItem = DbTreeItemFactory::createIndexes(this);
        triggersItem = DbTreeItemFactory::createTriggers(this);

        tableItem->appendRow(columnsItem);
        tableItem->appendRow(indexesItem);
        tableItem->appendRow(triggersItem);

        foreach (QStandardItem* columnItem, allTableColumns[tableItem->text()])
            columnsItem->appendRow(columnItem);

        foreach (QStandardItem* indexItem, indexes[tableItem->text()])
            indexesItem->appendRow(indexItem);

        foreach (QStandardItem* triggerItem, triggers[tableItem->text()])
            triggersItem->appendRow(triggerItem);
    }
    foreach (QStandardItem* viewItem, views)
    {
        viewsItem->appendRow(viewItem);

        triggersItem = DbTreeItemFactory::createTriggers(this);
        viewItem->appendRow(triggersItem);
        foreach (QStandardItem* triggerItem, triggers[viewItem->text()])
            triggersItem->appendRow(triggerItem);
    }
}

void DbTreeModel::restoreExpandedState(const QHash<QString, bool>& expandedState, QStandardItem* parentItem)
{
    DbTreeItem* parentDbTreeItem = dynamic_cast<DbTreeItem*>(parentItem);
    QString sig = parentDbTreeItem->signature();
    if (expandedState.contains(sig) && expandedState[sig])
        treeView->expand(parentItem->index());

    foreach (QStandardItem* child, parentDbTreeItem->childs())
        restoreExpandedState(expandedState, child);
}

void DbTreeModel::dbConnected(Db* db)
{
    QStandardItem* item = findItem(DbTreeItem::Type::DB, db);
    if (!item)
    {
        qWarning() << "Connected to db that couldn't be found in the model:" << db->getName();
        return;
    }
    refreshSchema(db, item);
    treeView->expand(item->index());
    if (CFG_UI.General.ExpandTables.get())
        treeView->expand(item->index().child(0, 0)); // also expand tables

    if (CFG_UI.General.ExpandViews.get())
        treeView->expand(item->index().child(1, 0)); // also expand views
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

void DbTreeModel::dbToBeUnloaded(Db* db, DbPlugin* plugin)
{
    UNUSED(plugin);

    DbTreeItem* item = findItem(DbTreeItem::Type::DB, db);

    // We need to close db now if it's open,
    // because after setting it to an invalid type,
    // it will have no db assigned and it will not react correctly
    // to a "disconnected" signal from DbManager.
    if (db->isOpen())
        db->close();

    if (item)
        item->setInvalidDbType(true);
}

void DbTreeModel::dbLoaded(Db* db, DbPlugin* plugin)
{
    UNUSED(plugin);

    DbTreeItem* item = findItem(DbTreeItem::Type::INVALID_DB, db->getName());
    if (item)
        item->setInvalidDbType(false, db);
}

void DbTreeModel::massSaveBegins()
{
    requireSchemaReloading = false;
}

void DbTreeModel::massSaveCommited()
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
    DbTreeItem* item;
    DbTreeItem* subItem;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
         item = dynamic_cast<DbTreeItem*>(parentItem->child(i));

         // Search recursively
         if (item->getType() == DbTreeItem::Type::DIR)
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

QList<DbTreeItem*> DbTreeModel::findItems(DbTreeItem::Type type)
{
    return findItems(root(), type);
}

DbTreeItem *DbTreeModel::findItem(QStandardItem* parentItem, DbTreeItem::Type type, Db* db)
{
    DbTreeItem* item;
    DbTreeItem* subItem;
    for (int i = 0; i < parentItem->rowCount(); i++)
    {
        item = dynamic_cast<DbTreeItem*>(parentItem->child(i));

        // Search recursively
        if (item->getType() == DbTreeItem::Type::DIR)
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
    DbTreeItem* item;
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

    QStandardItem* item;
    quint64 itemAddr;
    stream << reinterpret_cast<qint32>(indexes.size());
    for (const QModelIndex& idx : indexes)
    {
        item = itemFromIndex(idx);
        itemAddr = reinterpret_cast<quint64>(item);
        stream << itemAddr;
    }
    data->setData(MIMETYPE, output);

    return data;
}

bool DbTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    DbTreeItem* dstItem = nullptr;
    if (parent.isValid())
    {
        QModelIndex idx = parent.child(row, column);
        if (idx.isValid())
            dstItem = dynamic_cast<DbTreeItem*>(itemFromIndex(idx));
        else // drop on top of the parent
            dstItem = dynamic_cast<DbTreeItem*>(itemFromIndex(parent));
    }
    else
        dstItem = dynamic_cast<DbTreeItem*>(item(row, column));

    if (!dstItem)
        return QStandardItemModel::dropMimeData(data, action, row, column, parent);

    bool res = false;
    if (data->formats().contains(MIMETYPE))
        res = dropDbTreeItem(getDragItems(data), dstItem);
    else if (data->hasText())
        res = dropString(data->text(), dstItem);
    else if (data->hasUrls())
        res = dropUrls(data->urls(), dstItem);

    if (!res)
        return false;

    return QStandardItemModel::dropMimeData(data, action, row, column, parent);
}

QList<DbTreeItem*> DbTreeModel::getDragItems(const QMimeData* data)
{
    QList<DbTreeItem*> items;
    QByteArray byteData = data->data(MIMETYPE);
    QDataStream stream(&byteData, QIODevice::ReadOnly);

    qint32 itemCount;
    stream >> itemCount;

    quint64 itemAddr;
    for (qint32 i = 0; i < itemCount; i++)
    {
        stream >> itemAddr;
        items << dynamic_cast<DbTreeItem*>(reinterpret_cast<QStandardItem*>(itemAddr));
    }

    return items;
}

bool DbTreeModel::dropDbTreeItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem)
{
    if (srcItems.size() == 0)
        return false;

    DbTreeItem* srcItem = srcItems.first();
    switch (srcItem->getType())
    {
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::VIEW:
        {
            if (srcItem->getDb() == dstItem->getDb())
                return true;

            return dropDbObjectItem(srcItems, dstItem);
        }
        case DbTreeItem::Type::COLUMN:
//            return dropColumnItem(srcItems, dstItem); // TODO
        case DbTreeItem::Type::DIR:
        case DbTreeItem::Type::DB:
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::INDEX:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEWS:
        case DbTreeItem::Type::COLUMNS:
        case DbTreeItem::Type::INVALID_DB:
        case DbTreeItem::Type::VIRTUAL_TABLE:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }

    return true;
}

bool DbTreeModel::dropDbObjectItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem)
{
    moveOrCopyDbObjects(srcItems, dstItem);
    return false;
}

bool DbTreeModel::dropColumnItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem)
{
    return true;
}

bool DbTreeModel::dropString(const QString& str, DbTreeItem* dstItem)
{
    return true;
}

bool DbTreeModel::dropUrls(const QList<QUrl>& urls, DbTreeItem* dstItem)
{
    return true;
}

void DbTreeModel::moveOrCopyDbObjects(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem)
{
    if (srcItems.size() == 0)
        return;

    QMenu menu;
    menu.addAction(ICONS.ACT_COPY, tr("Copy"));
    QAction* moveAction = menu.addAction(ICONS.ACT_CUT, tr("Move"));

    QWidget* includeDataWidget = new QWidget(&menu);
    includeDataWidget->setLayout(new QHBoxLayout());
    QCheckBox *includeDataCheck = new QCheckBox(tr("Include data"));
    includeDataWidget->layout()->addWidget(includeDataCheck);
    QWidgetAction *includeDataAction = new QWidgetAction(&menu);
    includeDataAction->setDefaultWidget(includeDataWidget);
    includeDataCheck->setChecked(true);
    menu.addAction(includeDataAction);

    menu.addSeparator();
    QAction* abortAction = menu.addAction(ICONS.ACT_ABORT, tr("Abort"));

    bool abort = false;
    bool move = false;
    connect(moveAction, &QAction::triggered, [&move]() {move = true;});
    connect(abortAction, &QAction::triggered, [&abort]() {abort = true;});

    menu.exec(treeView->mapToGlobal(treeView->getLastDropPosition()));
    if (abort)
        return;

    bool includeData = includeDataCheck->isChecked();

    DbTreeItem* srcItem = srcItems.first();
    Db* srcDb = srcItem->getDb();
    Db* dstDb = dstItem->getDb();

    QStringList srcNames;
    for (DbTreeItem* item : srcItems)
        srcNames << item->text();

    if (move)
        dbOrganizer->moveObjectsToDb(srcDb, srcNames, dstDb, includeData);
    else
        dbOrganizer->copyObjectsToDb(srcDb, srcNames, dstDb, includeData);
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

void DbTreeModel::dbObjectsMoveFinished(bool success, Db* srcDb, Db* dstDb)
{
    if (!success)
        return;

    DBTREE->refreshSchema(srcDb);
    DBTREE->refreshSchema(dstDb);
}

void DbTreeModel::dbObjectsCopyFinished(bool success, Db* srcDb, Db* dstDb)
{
    UNUSED(srcDb);
    if (!success)
        return;

    DBTREE->refreshSchema(dstDb);
}
