#include "collationseditormodel.h"
#include "common/unused.h"
#include "common/strhash.h"
#include "services/pluginmanager.h"
#include "plugins/scriptingplugin.h"

CollationsEditorModel::CollationsEditorModel(QObject *parent) :
    QAbstractListModel(parent)
{
    init();
}

void CollationsEditorModel::clearModified()
{
    beginResetModel();
    foreach (Collation* coll, collationList)
        coll->modified = false;

    listModified = false;
    originalCollationList = collationList;

    endResetModel();
}

bool CollationsEditorModel::isModified() const
{
    if (collationList != originalCollationList)
        return true;

    foreach (Collation* coll, collationList)
    {
        if (coll->modified)
            return true;
    }
    return false;
}

bool CollationsEditorModel::isModified(int row) const
{
    if (!isValidRow(row))
        return false;

    return collationList[row]->modified;
}

void CollationsEditorModel::setModified(int row, bool modified)
{
    if (!isValidRow(row))
        return;

    collationList[row]->modified = modified;
    emitDataChanged(row);
}

void CollationsEditorModel::setName(int row, const QString& name)
{
    if (!isValidRow(row))
        return;

    collationList[row]->data->name = name;
    emitDataChanged(row);
}

QString CollationsEditorModel::getName(int row) const
{
    if (!isValidRow(row))
        return QString();

    return collationList[row]->data->name;
}

void CollationsEditorModel::setLang(int row, const QString& lang)
{
    if (!isValidRow(row))
        return;

    collationList[row]->data->lang = lang;
    emitDataChanged(row);
}

QString CollationsEditorModel::getLang(int row) const
{
    if (!isValidRow(row))
        return QString();

    return collationList[row]->data->lang;
}

void CollationsEditorModel::setAllDatabases(int row, bool allDatabases)
{
    if (!isValidRow(row))
        return;

    collationList[row]->data->allDatabases = allDatabases;
    emitDataChanged(row);
}

bool CollationsEditorModel::getAllDatabases(int row) const
{
    if (!isValidRow(row))
        return true;

    return collationList[row]->data->allDatabases;
}

void CollationsEditorModel::setCode(int row, const QString& code)
{
    if (!isValidRow(row))
        return;

    collationList[row]->data->code = code;
    emitDataChanged(row);
}

QString CollationsEditorModel::getCode(int row) const
{
    if (!isValidRow(row))
        return QString();

    return collationList[row]->data->code;
}

void CollationsEditorModel::setDatabases(int row, const QStringList& databases)
{
    if (!isValidRow(row))
        return;

    collationList[row]->data->databases = databases;
    emitDataChanged(row);
}

QStringList CollationsEditorModel::getDatabases(int row)
{
    if (!isValidRow(row))
        return QStringList();

    return collationList[row]->data->databases;
}

bool CollationsEditorModel::isValid(int row) const
{
    if (!isValidRow(row))
        return false;

    return collationList[row]->valid;
}

void CollationsEditorModel::setValid(int row, bool valid)
{
    if (!isValidRow(row))
        return;

    collationList[row]->valid = valid;
    emitDataChanged(row);
}

void CollationsEditorModel::setData(const QList<CollationManager::CollationPtr>& collations)
{
    beginResetModel();

    Collation* collationPtr;
    foreach (collationPtr, collationList)
        delete collationPtr;

    collationList.clear();

    foreach (const CollationManager::CollationPtr& coll, collations)
        collationList << new Collation(coll);

    listModified = false;
    originalCollationList = collationList;

    endResetModel();
}

void CollationsEditorModel::addCollation(const CollationManager::CollationPtr& collation)
{
    int row = collationList.size();

    beginInsertRows(QModelIndex(), row, row);

    collationList << new Collation(collation);
    listModified = true;

    endInsertRows();
}

void CollationsEditorModel::deleteCollation(int row)
{
    if (!isValidRow(row))
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete collationList[row];
    collationList.removeAt(row);

    listModified = true;

    endRemoveRows();
}

QList<CollationManager::CollationPtr> CollationsEditorModel::getCollations() const
{
    QList<CollationManager::CollationPtr> results;

    foreach (Collation* coll, collationList)
        results << coll->data;

    return results;
}

QStringList CollationsEditorModel::getCollationNames() const
{
    QStringList names;
    foreach (Collation* coll, collationList)
        names << coll->data->name;

    return names;
}

void CollationsEditorModel::validateNames()
{
    StrHash<QList<int>> counter;

    int row = 0;
    foreach (Collation* coll, collationList)
    {
        coll->valid = true;
        counter[coll->data->name] << row++;
    }

    QHashIterator<QString,QList<int>> cntIt = counter.iterator();
    while (cntIt.hasNext())
    {
        cntIt.next();
        if (cntIt.value().size() > 1)
        {
            foreach (int cntRow, cntIt.value())
                setValid(cntRow, false);
        }
    }

    QModelIndex idx;
    for (int i = 0; i < collationList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool CollationsEditorModel::isAllowedName(int rowToSkip, const QString& nameToValidate)
{
    QStringList names = getCollationNames();
    names.removeAt(rowToSkip);
    return !names.contains(nameToValidate, Qt::CaseInsensitive);
}

bool CollationsEditorModel::isValidRow(int row) const
{
    return (row >= 0 && row < collationList.size());
}

int CollationsEditorModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return collationList.size();
}

QVariant CollationsEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRow(index.row()))
        return QVariant();

    if (role == Qt::DisplayRole)
        return collationList[index.row()]->data->name;

    if (role == Qt::DecorationRole && langToIcon.contains(collationList[index.row()]->data->lang))
        return langToIcon[collationList[index.row()]->data->lang];

    return QVariant();

}

void CollationsEditorModel::init()
{
    QByteArray data;
    foreach (ScriptingPlugin* plugin, PLUGINS->getLoadedPlugins<ScriptingPlugin>())
    {
        data = QByteArray::fromBase64(plugin->getIconData());

        // The pixmap needs to be created per each iteration, so the pixmap is always loaded from scratch,
        // otherwise the first icon was used for all icons. It seems that loadFromData() appends the data
        // to the end of current data.
        QPixmap pixmap;
        if (pixmap.loadFromData(data))
            langToIcon[plugin->getLanguage()] = QIcon(pixmap);
    }
}

void CollationsEditorModel::emitDataChanged(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

CollationsEditorModel::Collation::Collation()
{
    data = CollationManager::CollationPtr::create();
}

CollationsEditorModel::Collation::Collation(const CollationManager::CollationPtr& other)
{
    data = CollationManager::CollationPtr::create(*other);
    originalName = data->name;
}
