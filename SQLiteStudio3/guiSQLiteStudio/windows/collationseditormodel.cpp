#include "collationseditormodel.h"
#include "common/unused.h"
#include "common/strhash.h"
#include "iconmanager.h"
#include "services/pluginmanager.h"
#include "plugins/scriptingplugin.h"
#include "icon.h"

#define SETTER(X, Y) \
    if (!isValidRowIndex(row) || X == Y) \
        return; \
    \
    X = Y; \
    emitDataChanged(row);

#define GETTER(X, Y) \
    if (!isValidRowIndex(row)) \
        return Y; \
    \
    return X;

CollationsEditorModel::CollationsEditorModel(QObject *parent) :
    QAbstractListModel(parent)
{
    init();
}

void CollationsEditorModel::clearModified()
{
    beginResetModel();
    for (Collation* coll : collationList)
        coll->modified = false;

    listModified = false;
    originalCollationList = collationList;

    endResetModel();
}

bool CollationsEditorModel::isModified() const
{
    if (collationList != originalCollationList)
        return true;

    for (Collation* coll : collationList)
    {
        if (coll->modified)
            return true;
    }
    return false;
}

bool CollationsEditorModel::isModified(int row) const
{
    GETTER(collationList[row]->modified, false);
}

void CollationsEditorModel::setModified(int row, bool modified)
{
    SETTER(collationList[row]->modified, modified);
}

void CollationsEditorModel::setName(int row, const QString& name)
{
    SETTER(collationList[row]->data->name, name);
}

QString CollationsEditorModel::getName(int row) const
{
    GETTER(collationList[row]->data->name, QString());
}

void CollationsEditorModel::setType(int row, CollationManager::CollationType type)
{
    SETTER(collationList[row]->data->type, type);
}

CollationManager::CollationType CollationsEditorModel::getType(int row) const
{
    GETTER(collationList[row]->data->type, CollationManager::CollationType::FUNCTION_BASED);
}

void CollationsEditorModel::setLang(int row, const QString& lang)
{
    SETTER(collationList[row]->data->lang, lang);
}

QString CollationsEditorModel::getLang(int row) const
{
    GETTER(collationList[row]->data->lang, QString());
}

void CollationsEditorModel::setAllDatabases(int row, bool allDatabases)
{
    SETTER(collationList[row]->data->allDatabases, allDatabases);
}

bool CollationsEditorModel::getAllDatabases(int row) const
{
    GETTER(collationList[row]->data->allDatabases, true);
}

void CollationsEditorModel::setCode(int row, const QString& code)
{
    SETTER(collationList[row]->data->code, code);
}

QString CollationsEditorModel::getCode(int row) const
{
    GETTER(collationList[row]->data->code, QString());
}

void CollationsEditorModel::setDatabases(int row, const QStringList& databases)
{
    SETTER(collationList[row]->data->databases, databases);
}

QStringList CollationsEditorModel::getDatabases(int row)
{
    GETTER(collationList[row]->data->databases, QStringList());
}

bool CollationsEditorModel::isValid(int row) const
{
    GETTER(collationList[row]->valid, false);
}

void CollationsEditorModel::setValid(int row, bool valid)
{
    SETTER(collationList[row]->valid, valid);
}

bool CollationsEditorModel::isValid() const
{
    for (Collation* coll : collationList)
    {
        if (!coll->valid)
            return false;
    }
    return true;
}

void CollationsEditorModel::setData(const QList<CollationManager::CollationPtr>& collations)
{
    beginResetModel();

    for (Collation* collationPtr : collationList)
        delete collationPtr;

    collationList.clear();

    for (const CollationManager::CollationPtr& coll : collations)
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
    if (!isValidRowIndex(row))
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
    for (Collation* coll : collationList)
        results << coll->data;

    return results;
}

QStringList CollationsEditorModel::getCollationNames() const
{
    QStringList names;
    for (Collation* coll : collationList)
        names << coll->data->name;

    return names;
}

void CollationsEditorModel::validateNames()
{
    StrHash<QList<int>> counter;

    int row = 0;
    for (Collation* coll : collationList)
    {
        coll->valid &= true;
        counter[coll->data->name] << row++;
    }

    QHashIterator<QString,QList<int>> cntIt = counter.iterator();
    while (cntIt.hasNext())
    {
        cntIt.next();
        if (cntIt.value().size() > 1)
        {
            for (int cntRow : cntIt.value())
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

bool CollationsEditorModel::isValidRowIndex(int row) const
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
    if (!index.isValid() || !isValidRowIndex(index.row()))
        return QVariant();

    if (role == Qt::DisplayRole)
        return collationList[index.row()]->data->name;

    if (role == Qt::DecorationRole)
    {
        auto coll = collationList[index.row()]->data;
        bool isExtension = coll->type == CollationManager::CollationType::EXTENSION_BASED;
        if (isExtension || langToIcon.contains(coll->lang))
        {
            return isExtension ? langToIcon[coll->lang] : ICONS.CONSTRAINT_COLLATION_ERROR;
        }
    }

    return QVariant();

}

void CollationsEditorModel::init()
{
    for (ScriptingPlugin* plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        langToIcon[plugin->getLanguage()] = QIcon(plugin->getIconPath());
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
