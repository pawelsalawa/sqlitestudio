#include "dbtreeitemfactory.h"
#include "iconmanager.h"
#include "unused.h"

DbTreeItem *DbTreeItemFactory::createDir(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::DIR, "directory", name, parent);
}

DbTreeItem *DbTreeItemFactory::createDb(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::DB, "database", name, parent);
}

DbTreeItem *DbTreeItemFactory::createTable(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::TABLE, "table", name, parent);
}

DbTreeItem *DbTreeItemFactory::createIndex(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::INDEX, "index", name, parent);
}

DbTreeItem *DbTreeItemFactory::createTrigger(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::TRIGGER, "trigger", name, parent);
}

DbTreeItem *DbTreeItemFactory::createView(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::VIEW, "view", name, parent);
}

DbTreeItem *DbTreeItemFactory::createColumn(const QString &name, QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::COLUMN, "column", name, parent);
}

DbTreeItem *DbTreeItemFactory::createTables(QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::TABLES, "tables", QObject::tr("Tables"), parent);
}

DbTreeItem *DbTreeItemFactory::createIndexes(QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::INDEXES, "indexes", QObject::tr("Indexes"), parent);
}

DbTreeItem *DbTreeItemFactory::createTriggers(QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::TRIGGERS, "triggers", QObject::tr("Triggers"), parent);
}

DbTreeItem *DbTreeItemFactory::createViews(QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::VIEWS, "views", QObject::tr("Views"), parent);
}

DbTreeItem *DbTreeItemFactory::createColumns(QObject *parent)
{
    return new DbTreeItem(DbTreeItem::Type::COLUMNS, "columns", QObject::tr("Columns"), parent);
}

DbTreeItem *DbTreeItemFactory::createPrototype(QObject *parent)
{
    UNUSED(parent);
    return new DbTreeItem();
}
