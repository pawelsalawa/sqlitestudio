#include "functionseditormodel.h"
#include "common/strhash.h"
#include "common/unused.h"
#include "services/pluginmanager.h"
#include "plugins/scriptingplugin.h"
#include "icon.h"
#include "iconmanager.h"
#include <QDebug>

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

FunctionsEditorModel::FunctionsEditorModel(QObject *parent) :
    QAbstractListModel(parent)
{
    init();
}

void FunctionsEditorModel::clearModified()
{
    beginResetModel();
    for (Function*& func : functionList)
        func->modified = false;

    listModified = false;
    originalFunctionList = functionList;

    endResetModel();
}

bool FunctionsEditorModel::isModified() const
{
    if (functionList != originalFunctionList)
        return true;

    for (Function* func : functionList)
    {
        if (func->modified)
            return true;
    }
    return false;
}

bool FunctionsEditorModel::isModified(int row) const
{
    GETTER(functionList[row]->modified, false);
}

void FunctionsEditorModel::setModified(int row, bool modified)
{
    SETTER(functionList[row]->modified, modified);
}

bool FunctionsEditorModel::isValid() const
{
    for (Function* func : functionList)
    {
        if (!func->valid)
            return false;
    }
    return true;
}

bool FunctionsEditorModel::isValid(int row) const
{
    GETTER(functionList[row]->valid, false);
}

void FunctionsEditorModel::setValid(int row, bool valid)
{
    SETTER(functionList[row]->valid, valid);
}

void FunctionsEditorModel::setCode(int row, const QString& code)
{
    SETTER(functionList[row]->data.code, code);
}

QString FunctionsEditorModel::getCode(int row) const
{
    GETTER(functionList[row]->data.code, QString());
}

void FunctionsEditorModel::setFinalCode(int row, const QString& code)
{
    SETTER(functionList[row]->data.finalCode, code);
}

QString FunctionsEditorModel::getFinalCode(int row) const
{
    GETTER(functionList[row]->data.finalCode, QString());
}

void FunctionsEditorModel::setInitCode(int row, const QString& code)
{
    SETTER(functionList[row]->data.initCode, code);
}

QString FunctionsEditorModel::getInitCode(int row) const
{
    GETTER(functionList[row]->data.initCode, QString());
}

void FunctionsEditorModel::setName(int row, const QString& newName)
{
    SETTER(functionList[row]->data.name, newName);
}

QString FunctionsEditorModel::getName(int row) const
{
    GETTER(functionList[row]->data.name, QString());
}

void FunctionsEditorModel::setLang(int row, const QString& lang)
{
    SETTER(functionList[row]->data.lang, lang);
}

QString FunctionsEditorModel::getLang(int row) const
{
    GETTER(functionList[row]->data.lang, QString());
}

bool FunctionsEditorModel::getUndefinedArgs(int row) const
{
    GETTER(functionList[row]->data.undefinedArgs, true);
}

void FunctionsEditorModel::setUndefinedArgs(int row, bool value)
{
    SETTER(functionList[row]->data.undefinedArgs, value);
}

bool FunctionsEditorModel::getAllDatabases(int row) const
{
    GETTER(functionList[row]->data.allDatabases, true);
}

void FunctionsEditorModel::setAllDatabases(int row, bool value)
{
    SETTER(functionList[row]->data.allDatabases, value);
}

FunctionManager::ScriptFunction::Type FunctionsEditorModel::getType(int row) const
{
    GETTER(functionList[row]->data.type, FunctionManager::ScriptFunction::SCALAR);
}

void FunctionsEditorModel::setType(int row, FunctionManager::ScriptFunction::Type type)
{
    SETTER(functionList[row]->data.type, type);
}

bool FunctionsEditorModel::isAggregate(int row) const
{
    GETTER(functionList[row]->data.type == FunctionManager::ScriptFunction::AGGREGATE, false);
}

bool FunctionsEditorModel::isScalar(int row) const
{
    GETTER(functionList[row]->data.type == FunctionManager::ScriptFunction::SCALAR, false);
}

void FunctionsEditorModel::setDeterministic(int row, bool value)
{
    SETTER(functionList[row]->data.deterministic, value);
}

bool FunctionsEditorModel::isDeterministic(int row) const
{
    GETTER(functionList[row]->data.deterministic, false);
}

QStringList FunctionsEditorModel::getArguments(int row) const
{
    GETTER(functionList[row]->data.arguments, QStringList());
}

void FunctionsEditorModel::setArguments(int row, const QStringList& value)
{
    SETTER(functionList[row]->data.arguments, value);
}

QStringList FunctionsEditorModel::getDatabases(int row) const
{
    GETTER(functionList[row]->data.databases, QStringList());
}

void FunctionsEditorModel::setDatabases(int row, const QStringList& value)
{
    SETTER(functionList[row]->data.databases, value);
}

void FunctionsEditorModel::setData(const QList<FunctionManager::ScriptFunction*>& functions)
{
    beginResetModel();

    for (Function*& functionPtr : functionList)
        delete functionPtr;

    functionList.clear();

    for (FunctionManager::ScriptFunction* func : functions)
        functionList << new Function(func);

    listModified = false;
    originalFunctionList = functionList;

    endResetModel();
}

void FunctionsEditorModel::addFunction(FunctionManager::ScriptFunction* function)
{
    int row = functionList.size();

    beginInsertRows(QModelIndex(), row, row);

    functionList << new Function(function);
    listModified = true;

    endInsertRows();
}

void FunctionsEditorModel::deleteFunction(int row)
{
    if (!isValidRowIndex(row))
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete functionList[row];
    functionList.removeAt(row);

    listModified = true;

    endRemoveRows();
}

QList<FunctionManager::ScriptFunction*> FunctionsEditorModel::generateFunctions() const
{
    QList<FunctionManager::ScriptFunction*> results;

    for (Function* func : functionList)
        results << new FunctionManager::ScriptFunction(func->data);

    return results;
}

QStringList FunctionsEditorModel::getFunctionNames() const
{
    QStringList names;
    for (Function* func : functionList)
        names << func->data.name;

    return names;
}

void FunctionsEditorModel::validateNames()
{
    QHash<UniqueFunctionName, QList<int>> counter;

    int row = 0;
    for (Function*& func : functionList)
    {
        func->valid &= true;
        UniqueFunctionName uniqueName = func->toUniqueName();
        counter[uniqueName] << row++;
    }

    QHashIterator<UniqueFunctionName, QList<int>> cntIt(counter);
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
    for (int i = 0; i < functionList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool FunctionsEditorModel::isAllowedName(int rowToSkip, const QString& nameToValidate, const QStringList& argList, bool undefinedArgs)
{
    QList<UniqueFunctionName> names = getUniqueFunctionNames();
    names.removeAt(rowToSkip);

    UniqueFunctionName validatedName;
    validatedName.name = nameToValidate.toLower();
    validatedName.undefArg = undefinedArgs;
    if (!undefinedArgs)
        validatedName.arguments = argList;

    return !names.contains(validatedName);
}

int FunctionsEditorModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return functionList.size();
}

QVariant FunctionsEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRowIndex(index.row()))
    {
        if (role == Qt::DecorationRole)
            return ICONS.FUNCTION_ERROR;

        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        Function* fn = functionList[index.row()];
        return fn->data.toString();
    }

    if (role == Qt::DecorationRole && langToIcon.contains(functionList[index.row()]->data.lang))
        return functionList[index.row()]->valid ? langToIcon[functionList[index.row()]->data.lang] : ICONS.FUNCTION_ERROR;

    if (role == Qt::DecorationRole)
        return ICONS.LIST_ITEM_OTHER;

    return QVariant();
}

void FunctionsEditorModel::init()
{
    for (ScriptingPlugin*& plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        langToIcon[plugin->getLanguage()] = QIcon(plugin->getIconPath());
}

bool FunctionsEditorModel::isValidRowIndex(int row) const
{
    return (row >= 0 && row < functionList.size());
}

void FunctionsEditorModel::emitDataChanged(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

QList<FunctionsEditorModel::UniqueFunctionName> FunctionsEditorModel::getUniqueFunctionNames() const
{
    QList<UniqueFunctionName> names;
    for (Function* func : functionList)
        names << func->toUniqueName();

    return names;
}

FunctionsEditorModel::Function::Function()
{
}

FunctionsEditorModel::Function::Function(FunctionManager::ScriptFunction* other)
{
    data = FunctionManager::ScriptFunction(*other);
    originalName = data.name;
}

FunctionsEditorModel::UniqueFunctionName FunctionsEditorModel::Function::toUniqueName() const
{
    UniqueFunctionName uniqName;
    uniqName.name = data.name.toLower();
    uniqName.undefArg = data.undefinedArgs;
    if (!data.undefinedArgs)
        uniqName.arguments = data.arguments;

    return uniqName;
}

int FunctionsEditorModel::UniqueFunctionName::argCount() const
{
    return undefArg ? -1 : arguments.size();
}

bool FunctionsEditorModel::UniqueFunctionName::operator==(const UniqueFunctionName &other) const
{
    return name == other.name && argCount() == other.argCount();
}

int qHash(FunctionsEditorModel::UniqueFunctionName fnName)
{
    return qHash(fnName.name) ^ fnName.argCount();
}
