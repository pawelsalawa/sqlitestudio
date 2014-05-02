#include "dbtreeitem.h"
#include "iconmanager.h"
#include "dbtreemodel.h"
#include "services/dbmanager.h"
#include "dbtree.h"
#include <QDebug>

DbTreeItem::DbTreeItem(DbTreeItem::Type type, const Icon& icon, const QString& nodeName, QObject* parent)
    : DbTreeItem(type, nodeName, parent)
{
    setIcon(icon);
}

DbTreeItem::DbTreeItem(DbTreeItem::Type type, const QString& nodeName, QObject *parent)
    : QObject(parent)
{
    setText(nodeName);
    setType(type);
    init();
}

DbTreeItem::DbTreeItem(const DbTreeItem& item)
    : QObject(item.QObject::parent()), QStandardItem(item)
{
    init();
}

DbTreeItem::DbTreeItem()
{
    setType(Type::ITEM_PROTOTYPE);
    init();
}

void DbTreeItem::initMeta()
{
    qRegisterMetaType<DbTreeItem*>("DbTreeItem*");
    qRegisterMetaTypeStreamOperators<DbTreeItem*>("DbTreeItem*");
}

DbTreeItem::Type DbTreeItem::getType() const
{
    return static_cast<Type>(type());
}

void DbTreeItem::setType(Type type)
{
    setData(static_cast<int>(type), DataRole::TYPE);
}

int DbTreeItem::type() const
{
    return data(DataRole::TYPE).toInt();
}

QStandardItem *DbTreeItem::findItem(DbTreeItem::Type type, const QString& name)
{
    return DbTreeModel::findItem(this, type, name);
}

QStandardItem* DbTreeItem::clone() const
{
    return new DbTreeItem(*this);
}

QList<QStandardItem *> DbTreeItem::childs() const
{
    QList<QStandardItem *> results;
    for (int i = 0; i < rowCount(); i++)
         results += child(i);

    return results;
}

QStringList DbTreeItem::childNames() const
{
    QStringList results;
    for (int i = 0; i < rowCount(); i++)
         results += child(i)->text();

    return results;
}

QString DbTreeItem::getTable() const
{
    const DbTreeItem* item = getParentItem(Type::TABLE);
    if (!item)
        return QString::null;

    return item->text();
}

QString DbTreeItem::getColumn() const
{
    if (getType() != Type::COLUMN)
        return QString::null;

    return text();
}

QString DbTreeItem::getIndex() const
{
    const DbTreeItem* item = getParentItem(Type::INDEX);
    if (!item)
        return QString::null;

    return item->text();
}

QString DbTreeItem::getTrigger() const
{
    const DbTreeItem* item = getParentItem(Type::TRIGGER);
    if (!item)
        return QString::null;

    return item->text();
}

QString DbTreeItem::getView() const
{
    const DbTreeItem* item = getParentItem(Type::VIEW);
    if (!item)
        return QString::null;

    return item->text();
}

QStandardItem *DbTreeItem::parentItem() const
{
    if (!QStandardItem::parent())
        return model()->invisibleRootItem();

    return QStandardItem::parent();
}

DbTreeItem *DbTreeItem::parentDbTreeItem() const
{
    QStandardItem* parentItem = QStandardItem::parent();
    if (!parentItem)
        return nullptr;

    return dynamic_cast<DbTreeItem*>(parentItem);
}

QList<DbTreeItem *> DbTreeItem::getPathToRoot()
{
    QList<DbTreeItem *> path;
    getPathToRoot(path);
    return path;
}

QList<DbTreeItem*> DbTreeItem::getPathToParentItem(DbTreeItem::Type type)
{
    QList<DbTreeItem*> path;
    getPathToParentItem(path, type);
    return path;
}

QList<DbTreeItem*> DbTreeItem::getPathToParentItem(DbTreeItem::Type type, const QString& name)
{
    QList<DbTreeItem*> path;
    getPathToParentItem(path, type, name);
    return path;
}

DbTreeItem* DbTreeItem::findParentItem(DbTreeItem::Type type)
{
    DbTreeItem* parent = parentDbTreeItem();
    if (!parent)
        return nullptr;

    if (parent->getType() == type)
        return parent;

    return parent->findParentItem(type);
}

DbTreeItem* DbTreeItem::findParentItem(DbTreeItem::Type type, const QString& name)
{
    DbTreeItem* parent = parentDbTreeItem();
    if (!parent)
        return nullptr;

    if (parent->getType() == type && name == parent->text())
        return parent;

    return parent->findParentItem(type);
}

void DbTreeItem::getPathToRoot(QList<DbTreeItem *> &path)
{
    path << this;
    if (parentDbTreeItem())
        parentDbTreeItem()->getPathToRoot(path);
}

QString DbTreeItem::signature()
{
    QString sig;
    if (parentDbTreeItem())
        sig += parentDbTreeItem()->signature();

    sig += "_"+QString::number(type())+text();
    return sig;
}

void DbTreeItem::getPathToParentItem(QList<DbTreeItem*>& path, DbTreeItem::Type type)
{
    path << this;
    if (getType() == type)
        return;

    if (parentDbTreeItem())
        parentDbTreeItem()->getPathToParentItem(path, type);
}

void DbTreeItem::getPathToParentItem(QList<DbTreeItem*>& path, DbTreeItem::Type type, const QString& name)
{
    path << this;
    if (getType() == type && name == text())
        return;

    if (parentDbTreeItem())
        parentDbTreeItem()->getPathToParentItem(path, type, name);
}

const DbTreeItem* DbTreeItem::getParentItem(DbTreeItem::Type type) const
{
    if (getType() == type)
        return this;

    DbTreeItem* parent = parentDbTreeItem();
    if (parent)
        return parent->getParentItem(Type::TABLE);

    return nullptr;
}

Db* DbTreeItem::getDb() const
{
    QString dbName = data(DataRole::DB).toString();
    return DBLIST->getByName(dbName);
}

void DbTreeItem::setDb(Db* value)
{
    if (!value)
    {
        setDb(QString::null);
        return;
    }

    setDb(value->getName());
}

void DbTreeItem::setDb(const QString& dbName)
{
    setData(dbName, DataRole::DB);
}

const Icon* DbTreeItem::getIcon() const
{
    return data(DataRole::ICON_PTR).value<const Icon*>();
}

void DbTreeItem::setHidden(bool hidden)
{
    setData(hidden, DataRole::HIDDEN);
    dynamic_cast<DbTreeModel*>(model())->itemChangedVisibility(this);
}

bool DbTreeItem::isHidden() const
{
    return data(DataRole::HIDDEN).toBool();
}

void DbTreeItem::setIcon(const Icon& icon)
{
    setData(QVariant::fromValue(&icon), DataRole::ICON_PTR);
    if (!icon.isNull())
        QStandardItem::setIcon(icon);
}

void DbTreeItem::setInvalidDbType(bool invalid, Db* db)
{
    if (invalid)
    {
        setDb(nullptr);
        setType(DbTreeItem::Type::INVALID_DB);
        setIcon(ICONS.DATABASE_INVALID);
    }
    else
    {
        setDb(db);
        setType(DbTreeItem::Type::DB);
        setIcon(ICONS.DATABASE);
    }
}

void DbTreeItem::init()
{
    Type type = getType();
    if (type == Type::DIR)
        setEditable(true);
    else
        setEditable(false);

    setData(false, DataRole::HIDDEN);

    Qt::ItemFlags f = flags();
    if (DbTree::isItemDraggable(this))
        f |= Qt::ItemIsDragEnabled;
    else
        f ^= Qt::ItemIsDragEnabled;

    setFlags(f);
}

QDataStream &operator <<(QDataStream &out, const DbTreeItem *item)
{
    out << reinterpret_cast<quint64>(item);
    return out;
}

QDataStream &operator >>(QDataStream &in, DbTreeItem *&item)
{
    quint64 ptr;
    in >> ptr;
    item = reinterpret_cast<DbTreeItem*>(ptr);
    return in;
}

int qHash(DbTreeItem::Type type)
{
    return static_cast<int>(type);
}
