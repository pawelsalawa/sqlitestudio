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

    endResetModel();
}

bool FunctionsEditorModel::isModified() const
{
    if (listModified)
        return true;

    foreach (Function* func, functionList)
    {
        if (func->modified)
            return true;
    }
    return false;
}

bool FunctionsEditorModel::isModified(const QString& name) const
{
    if (!functionMap.contains(name))
        return false;

    return functionMap[name]->modified;
}

void FunctionsEditorModel::setModified(const QString& name, bool modified)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->modified = modified;
    emitDataChanged(name);
}

bool FunctionsEditorModel::isValid(const QString& name) const
{
    if (!functionMap.contains(name))
        return false;

    return functionMap[name]->valid;
}

void FunctionsEditorModel::setValid(const QString& name, bool valid)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->valid = valid;
    emitDataChanged(name);
}

void FunctionsEditorModel::setCode(const QString& name, const QString& code)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->code = code;
    emitDataChanged(name);
}

QString FunctionsEditorModel::getCode(const QString& name) const
{
    if (!functionMap.contains(name))
        return QString::null;

    return functionMap[name]->code;
}

void FunctionsEditorModel::setName(const QString& name, const QString& newName)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->name = newName;
    emitDataChanged(name);
}

QString FunctionsEditorModel::getName(const QString& name) const
{
    if (!functionMap.contains(name))
        return QString::null;

    return functionMap[name]->name;
}

void FunctionsEditorModel::setLang(const QString& name, const QString& lang)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->lang = lang;
    emitDataChanged(name);
}

QString FunctionsEditorModel::getLang(const QString& name) const
{
    if (!functionMap.contains(name))
        return QString::null;

    return functionMap[name]->lang;
}

bool FunctionsEditorModel::getUndefinedArgs(const QString& name) const
{
    if (!functionMap.contains(name))
        return true;

    return functionMap[name]->undefinedArgs;
}

void FunctionsEditorModel::setUndefinedArgs(const QString& name, bool value)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->undefinedArgs = value;
    emitDataChanged(name);
}

bool FunctionsEditorModel::getAllDatabases(const QString& name) const
{
    if (!functionMap.contains(name))
        return true;

    return functionMap[name]->allDatabases;
}

void FunctionsEditorModel::setAllDatabases(const QString& name, bool value)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->allDatabases = value;
    emitDataChanged(name);
}

QString FunctionsEditorModel::getType(const QString& name) const
{
    if (!functionMap.contains(name))
        return Config::Function::SCALAR_TYPE;

    switch (functionMap[name]->type)
    {
        case FunctionsEditorModel::Function::SCALAR:
            return Config::Function::SCALAR_TYPE;
        case FunctionsEditorModel::Function::AGGREGATE:
            return Config::Function::AGGREGATE_TYPE;
    }
    return Config::Function::SCALAR_TYPE;
}

void FunctionsEditorModel::setType(const QString& name, const QString& value)
{
    if (!functionMap.contains(name))
        return;

    if (value == Config::Function::AGGREGATE_TYPE)
        functionMap[name]->type = Function::AGGREGATE;
    else
        functionMap[name]->type = Function::SCALAR;

    emitDataChanged(name);
}

QStringList FunctionsEditorModel::getArguments(const QString& name) const
{
    if (!functionMap.contains(name))
        return QStringList();

    return functionMap[name]->arguments;
}

void FunctionsEditorModel::setArguments(const QString& name, const QStringList& value)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->arguments = value;
    emitDataChanged(name);
}

QStringList FunctionsEditorModel::getDatabases(const QString& name) const
{
    if (!functionMap.contains(name))
        return QStringList();

    return functionMap[name]->databases;
}

void FunctionsEditorModel::setDatabases(const QString& name, const QStringList& value)
{
    if (!functionMap.contains(name))
        return;

    functionMap[name]->databases = value;
    emitDataChanged(name);
}

void FunctionsEditorModel::setData(const QList<Config::Function>& functions)
{
    beginResetModel();

    Function* functionPtr;
    foreach (functionPtr, functionList)
        delete functionPtr;

    functionList.clear();
    functionMap.clear();

    foreach (const Config::Function& func, functions)
    {
        functionPtr = new Function(func);
        functionMap[func.name] = functionPtr;
    }
    updateSortedList();

    listModified = false;

    endResetModel();
}

void FunctionsEditorModel::addFunction(const Config::Function& function)
{
    beginResetModel();

    Function* functionPtr = new Function(function);
    functionMap[function.name] = functionPtr;
    updateSortedList();

    listModified = true;

    endResetModel();
}

void FunctionsEditorModel::deleteFunction(const QString& name)
{
    if (!functionMap.contains(name))
        return;

    beginResetModel();

    Function* functionPtr = functionMap[name];
    functionList.removeOne(functionPtr);
    functionMap.remove(name);
    delete functionPtr;

    listModified = true;

    endResetModel();
}

void FunctionsEditorModel::updateFunction(const QString& name, const Config::Function& function)
{
    if (!functionMap.contains(name))
        return;

    Function* functionPtr = functionMap[name];
    functionPtr->name = function.name;
    functionPtr->lang = function.lang;
    functionPtr->code = function.code;

    emitDataChanged(name);
}

QList<Config::Function> FunctionsEditorModel::getConfigFunctions() const
{
    QList<Config::Function> configFunctions;

    foreach (Function* func, functionList)
        configFunctions << func->toConfigFunction();

    return configFunctions;
}

QModelIndex FunctionsEditorModel::indexOf(const QString& name)
{
    int i = 0;
    foreach (Function* func, functionList)
    {
        if (func->name == name)
            return index(i);

        i++;
    }

    return QModelIndex();
}

QStringList FunctionsEditorModel::getFunctionNames() const
{
    return functionMap.keys();
}

void FunctionsEditorModel::validateNames()
{
    StrHash<QStringList> counter;

    QHashIterator<QString,Function*> it(functionMap);
    while (it.hasNext())
    {
        it.next();
        it.value()->valid = true;
        counter[it.value()->name] << it.key();
    }

    QHashIterator<QString,QStringList> cntIt = counter.iterator();
    while (cntIt.hasNext())
    {
        cntIt.next();
        if (cntIt.value().size() > 1)
        {
            foreach (const QString& name, cntIt.value())
                setValid(name, false);
        }
    }

    QModelIndex idx;
    for (int i = 0; i < functionList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool FunctionsEditorModel::isAllowedName(const QString& name, const QString& nameToValidate)
{
    QStringList names;
    QHashIterator<QString,Function*> it(functionMap);
    while (it.hasNext())
    {
        it.next();
        if (it.key() == name)
            continue;

        names << it.value()->name;
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
    if (!index.isValid() || !isValidIndex(index.row()))
        return QVariant();

    if (role == Qt::DisplayRole)
        return functionList[index.row()]->name;

    if (role == Qt::DecorationRole && langToIcon.contains(functionList[index.row()]->lang))
        return langToIcon[functionList[index.row()]->lang];

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

bool FunctionsEditorModel::isValidIndex(int row) const
{
    return (row >= 0 && row < functionList.size());
}

void FunctionsEditorModel::updateSortedList()
{
    functionList.clear();
    QStringList names = functionMap.keys();
    qSort(names);
    foreach (const QString& name, names)
        functionList << functionMap[name];
}

void FunctionsEditorModel::emitDataChanged(const QString& name)
{
    QModelIndex idx = indexOf(name);
    emit dataChanged(idx, idx);
}

FunctionsEditorModel::Function::Function()
{
}

FunctionsEditorModel::Function::Function(const Config::Function& other)
{
    name = other.name;
    lang = other.lang;
    code = other.code;
    databases = other.databases;
    allDatabases = other.allDatabases;
    arguments = other.arguments;
    undefinedArgs = other.undefinedArgs;

    if (other.aggregate)
        type = AGGREGATE;
    else
        type = SCALAR;
}

Config::Function FunctionsEditorModel::Function::toConfigFunction() const
{
    Config::Function func;
    func.name = name;
    func.lang = lang;
    func.code = code;
    func.databases = databases;
    func.allDatabases = allDatabases;
    func.arguments = arguments;
    func.undefinedArgs = undefinedArgs;
    func.aggregate = (type == AGGREGATE);
    return func;
}
