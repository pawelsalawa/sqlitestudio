#include "sqliteextensioneditormodel.h"
#include "iconmanager.h"

#include <QFileInfo>

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

SqliteExtensionEditorModel::SqliteExtensionEditorModel(QObject* parent) :
    QAbstractListModel(parent)
{
}

void SqliteExtensionEditorModel::clearModified()
{
    beginResetModel();
    for (Extension* ext : extensionList)
        ext->modified = false;

    listModified = false;
    originalExtensionList = extensionList;
    endResetModel();
}

bool SqliteExtensionEditorModel::isModified() const
{
    if (extensionList != originalExtensionList)
        return true;

    for (Extension* ext : extensionList)
    {
        if (ext->modified)
            return true;
    }
    return false;
}

bool SqliteExtensionEditorModel::isModified(int row) const
{
    GETTER(extensionList[row]->modified, false);
}

void SqliteExtensionEditorModel::setModified(int row, bool modified)
{
    SETTER(extensionList[row]->modified, modified);
}

QString SqliteExtensionEditorModel::getName(int row) const
{
    GETTER(extensionList[row]->name, QString());
}

void SqliteExtensionEditorModel::setName(int row, const QString& name)
{
    SETTER(extensionList[row]->name, name);

    QModelIndex idx = index(0);
    emit dataChanged(idx, idx, {Qt::DisplayRole});
}

void SqliteExtensionEditorModel::setFilePath(int row, const QString& filePath)
{
    SETTER(extensionList[row]->data->filePath, filePath);
}

QString SqliteExtensionEditorModel::getFilePath(int row) const
{
    GETTER(extensionList[row]->data->filePath, QString());
}

void SqliteExtensionEditorModel::setInitFunction(int row, const QString& initFunc)
{
    SETTER(extensionList[row]->data->initFunc, initFunc);
}

QString SqliteExtensionEditorModel::getInitFunction(int row) const
{
    GETTER(extensionList[row]->data->initFunc, QString());
}

void SqliteExtensionEditorModel::setAllDatabases(int row, bool allDatabases)
{
    SETTER(extensionList[row]->data->allDatabases, allDatabases);
}

bool SqliteExtensionEditorModel::getAllDatabases(int row) const
{
    GETTER(extensionList[row]->data->allDatabases, true);
}

void SqliteExtensionEditorModel::setDatabases(int row, const QStringList& databases)
{
    SETTER(extensionList[row]->data->databases, databases);
}

QStringList SqliteExtensionEditorModel::getDatabases(int row)
{
    GETTER(extensionList[row]->data->databases, QStringList());
}

bool SqliteExtensionEditorModel::isValid(int row) const
{
    GETTER(extensionList[row]->valid, true);
}

void SqliteExtensionEditorModel::setValid(int row, bool valid)
{
    SETTER(extensionList[row]->valid, valid);

    QModelIndex idx = index(0);
    emit dataChanged(idx, idx, {Qt::DecorationRole});
}

bool SqliteExtensionEditorModel::isValid() const
{
    for (Extension* ext : extensionList)
    {
        if (ext->modified && !ext->valid)
            return false;
    }
    return true;
}

void SqliteExtensionEditorModel::setData(const QList<SqliteExtensionManager::ExtensionPtr>& extensions)
{
    beginResetModel();

    for (Extension* extPtr : extensionList)
        delete extPtr;

    extensionList.clear();

    for (const SqliteExtensionManager::ExtensionPtr& ext : extensions)
        extensionList << new Extension(ext);

    listModified = false;
    originalExtensionList = extensionList;

    endResetModel();
}

void SqliteExtensionEditorModel::addExtension(const SqliteExtensionManager::ExtensionPtr& extension)
{
    int row = extensionList.size();

    beginInsertRows(QModelIndex(), row, row);

    extensionList << new Extension(extension);
    listModified = true;

    endInsertRows();
}

void SqliteExtensionEditorModel::deleteExtension(int row)
{
    if (!isValidRowIndex(row))
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete extensionList[row];
    extensionList.removeAt(row);

    listModified = true;

    endRemoveRows();
}

QList<SqliteExtensionManager::ExtensionPtr> SqliteExtensionEditorModel::getExtensions() const
{
    QList<SqliteExtensionManager::ExtensionPtr> results;
    for (Extension* ext : extensionList)
        results << ext->data;

    return results;
}

bool SqliteExtensionEditorModel::isValidRowIndex(int row) const
{
    return (row >= 0 && row < extensionList.size());
}

int SqliteExtensionEditorModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return extensionList.size();
}

int SqliteExtensionEditorModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant SqliteExtensionEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRowIndex(index.row()))
        return QVariant();

    switch (index.column())
    {
        case 0:
        {
            if (role == Qt::DisplayRole)
                return getName(index.row());

            if (role == Qt::DecorationRole)
                return isValid(index.row()) ? ICONS.EXTENSION : ICONS.EXTENSION_ERROR;

            break;
        }
        case 1:
        {
            if (role == Qt::DisplayRole)
            {
                auto ext = extensionList[index.row()];
                return ext->data->allDatabases ? "*" : QString::number(ext->data->databases.size());
            }
            break;
        }
    }

    return QVariant();
}

void SqliteExtensionEditorModel::emitDataChanged(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

SqliteExtensionEditorModel::Extension::Extension()
{
    data = SqliteExtensionManager::ExtensionPtr::create();
}

SqliteExtensionEditorModel::Extension::Extension(const SqliteExtensionManager::ExtensionPtr& other)
{
    data = SqliteExtensionManager::ExtensionPtr::create(*other);
}
