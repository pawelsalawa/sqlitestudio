#include "functionseditormodel.h"
#include "strhash.h"
#include "unused.h"
#include "pluginmanager.h"
#include "sqlfunctionplugin.h"
#include <QDebug>

FunctionsEditorModel::FunctionsEditorModel(QObject *parent) :
    QAbstractListModel(parent)
{
    init();
}

void FunctionsEditorModel::clearModified()
{
    beginResetModel();
    foreach (Function* func, functionList)
        func->modified = false;

    listModified = false;
    originalFunctionList = functionList;

    endResetModel();
}

bool FunctionsEditorModel::isModified() const
{
    if (functionList != originalFunctionList)
        return true;

    foreach (Function* func, functionList)
    {
        if (func->modified)
            return true;
    }
    return false;
}

bool FunctionsEditorModel::isModified(int row) const
{
    if (!isValidRow(row))
        return false;

    return functionList[row]->modified;
}

void FunctionsEditorModel::setModified(int row, bool modified)
{
    if (!isValidRow(row))
        return;

    functionList[row]->modified = modified;
    emitDataChanged(row);
}

bool FunctionsEditorModel::isValid(int row) const
{
    if (!isValidRow(row))
        return false;

    return functionList[row]->valid;
}

void FunctionsEditorModel::setValid(int row, bool valid)
{
    if (!isValidRow(row))
        return;

    functionList[row]->valid = valid;
    emitDataChanged(row);
}

void FunctionsEditorModel::setCode(int row, const QString& code)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->code = code;
    emitDataChanged(row);
}

QString FunctionsEditorModel::getCode(int row) const
{
    if (!isValidRow(row))
        return QString::null;

    return functionList[row]->data->code;
}

void FunctionsEditorModel::setFinalCode(int row, const QString& code)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->finalCode = code;
    emitDataChanged(row);
}

QString FunctionsEditorModel::getFinalCode(int row) const
{
    if (!isValidRow(row))
        return QString::null;

    return functionList[row]->data->finalCode;
}

void FunctionsEditorModel::setName(int row, const QString& newName)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->name = newName;
    emitDataChanged(row);
}

QString FunctionsEditorModel::getName(int row) const
{
    if (!isValidRow(row))
        return QString::null;

    return functionList[row]->data->name;
}

void FunctionsEditorModel::setLang(int row, const QString& lang)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->lang = lang;
    emitDataChanged(row);
}

QString FunctionsEditorModel::getLang(int row) const
{
    if (!isValidRow(row))
        return QString::null;

    return functionList[row]->data->lang;
}

bool FunctionsEditorModel::getUndefinedArgs(int row) const
{
    if (!isValidRow(row))
        return true;

    return functionList[row]->data->undefinedArgs;
}

void FunctionsEditorModel::setUndefinedArgs(int row, bool value)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->undefinedArgs = value;
    emitDataChanged(row);
}

bool FunctionsEditorModel::getAllDatabases(int row) const
{
    if (!isValidRow(row))
        return true;

    return functionList[row]->data->allDatabases;
}

void FunctionsEditorModel::setAllDatabases(int row, bool value)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->allDatabases = value;
    emitDataChanged(row);
}

FunctionManager::Function::Type FunctionsEditorModel::getType(int row) const
{
    if (!isValidRow(row))
        return FunctionManager::Function::SCALAR;

    return functionList[row]->data->type;
}

void FunctionsEditorModel::setType(int row, FunctionManager::Function::Type type)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->type = type;
    emitDataChanged(row);
}

bool FunctionsEditorModel::isAggregate(int row) const
{
    if (!isValidRow(row))
        return false;

    return functionList[row]->data->type == FunctionManager::Function::AGGREGATE;
}

bool FunctionsEditorModel::isScalar(int row) const
{
    if (!isValidRow(row))
        return false;

    return functionList[row]->data->type == FunctionManager::Function::SCALAR;
}

QStringList FunctionsEditorModel::getArguments(int row) const
{
    if (!isValidRow(row))
        return QStringList();

    return functionList[row]->data->arguments;
}

void FunctionsEditorModel::setArguments(int row, const QStringList& value)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->arguments = value;
    emitDataChanged(row);
}

QStringList FunctionsEditorModel::getDatabases(int row) const
{
    if (!isValidRow(row))
        return QStringList();

    return functionList[row]->data->databases;
}

void FunctionsEditorModel::setDatabases(int row, const QStringList& value)
{
    if (!isValidRow(row))
        return;

    functionList[row]->data->databases = value;
    emitDataChanged(row);
}

void FunctionsEditorModel::setData(const QList<FunctionManager::FunctionPtr>& functions)
{
    beginResetModel();

    Function* functionPtr;
    foreach (functionPtr, functionList)
        delete functionPtr;

    functionList.clear();

    foreach (const FunctionManager::FunctionPtr& func, functions)
        functionList << new Function(func);

    listModified = false;
    originalFunctionList = functionList;

    endResetModel();
}

void FunctionsEditorModel::addFunction(const FunctionManager::FunctionPtr& function)
{
    int row = functionList.size();

    beginInsertRows(QModelIndex(), row, row);

    functionList << new Function(function);
    listModified = true;

    endInsertRows();
}

void FunctionsEditorModel::deleteFunction(int row)
{
    if (!isValidRow(row))
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete functionList[row];
    functionList.removeAt(row);

    listModified = true;

    endRemoveRows();
}

QList<FunctionManager::FunctionPtr> FunctionsEditorModel::getFunctions() const
{
    QList<FunctionManager::FunctionPtr> results;

    foreach (Function* func, functionList)
        results << func->data;

    return results;
}

QStringList FunctionsEditorModel::getFunctionNames() const
{
    QStringList names;
    foreach (Function* func, functionList)
        names << func->data->name;

    return names;
}

void FunctionsEditorModel::validateNames()
{
    StrHash<QList<int>> counter;

    int row = 0;
    foreach (Function* func, functionList)
    {
        func->valid = true;
        counter[func->data->name] << row++;
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
    for (int i = 0; i < functionList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool FunctionsEditorModel::isAllowedName(int rowToSkip, const QString& nameToValidate)
{
    QStringList names;
    int row = -1;
    foreach (Function* func, functionList)
    {
        row++;
        if (row == rowToSkip)
            continue;

        names << func->data->name;
    }
    return !names.contains(nameToValidate, Qt::CaseInsensitive);
}

int FunctionsEditorModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return functionList.size();
}

QVariant FunctionsEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRow(index.row()))
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        static const QString format = "%1(%2)";
        Function* fn = functionList[index.row()];
        QString args = fn->data->undefinedArgs ? "..." : fn->data->arguments.join(", ");
        return format.arg(fn->data->name).arg(args);
    }

    if (role == Qt::DecorationRole && langToIcon.contains(functionList[index.row()]->data->lang))
        return langToIcon[functionList[index.row()]->data->lang];

    return QVariant();
}

void FunctionsEditorModel::init()
{
    QPixmap pixmap;
    QByteArray data;
    foreach (SqlFunctionPlugin* plugin, PLUGINS->getLoadedPlugins<SqlFunctionPlugin>())
    {
        data = QByteArray::fromBase64(plugin->getIconData());
        if (pixmap.loadFromData(data))
            langToIcon[plugin->getLanguageName()] = QIcon(pixmap);
    }
}

bool FunctionsEditorModel::isValidRow(int row) const
{
    return (row >= 0 && row < functionList.size());
}

void FunctionsEditorModel::emitDataChanged(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

FunctionsEditorModel::Function::Function()
{
    data = FunctionManager::FunctionPtr::create();
}

FunctionsEditorModel::Function::Function(const FunctionManager::FunctionPtr& other)
{
    data = FunctionManager::FunctionPtr::create(*other);
    originalName = data->name;
}
