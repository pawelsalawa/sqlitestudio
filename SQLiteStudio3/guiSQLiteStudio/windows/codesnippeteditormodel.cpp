#include "codesnippeteditormodel.h"
#include "common/strhash.h"
#include "common/unused.h"
#include "iconmanager.h"

#include <QKeySequence>

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

CodeSnippetEditorModel::CodeSnippetEditorModel(QObject* parent) :
    QAbstractListModel(parent)
{
}

void CodeSnippetEditorModel::clearModified()
{
    beginResetModel();
    for (Snippet*& snip : snippetList)
        snip->modified = false;

    listModified = false;
    originalSnippetList = snippetList;

    endResetModel();
}

bool CodeSnippetEditorModel::isModified() const
{
    if (snippetList != originalSnippetList)
        return true;

    for (Snippet* snip : snippetList)
    {
        if (snip->modified)
            return true;
    }
    return false;
}

bool CodeSnippetEditorModel::isModified(int row) const
{
    GETTER(snippetList[row]->modified, false);
}

void CodeSnippetEditorModel::setModified(int row, bool modified)
{
    SETTER(snippetList[row]->modified, modified);
}

bool CodeSnippetEditorModel::isValid() const
{
    for (Snippet* snip : snippetList)
    {
        if (!snip->valid)
            return false;
    }
    return true;
}

bool CodeSnippetEditorModel::isValid(int row) const
{
    GETTER(snippetList[row]->valid, false);
}

void CodeSnippetEditorModel::setValid(int row, bool valid)
{
    SETTER(snippetList[row]->valid, valid);
}

void CodeSnippetEditorModel::setCode(int row, const QString& code)
{
    SETTER(snippetList[row]->data.code, code);
}

QString CodeSnippetEditorModel::getCode(int row) const
{
    GETTER(snippetList[row]->data.code, QString());
}

void CodeSnippetEditorModel::setName(int row, const QString& newName)
{
    SETTER(snippetList[row]->data.name, newName);
}

QString CodeSnippetEditorModel::getName(int row) const
{
    GETTER(snippetList[row]->data.name, QString());
}

void CodeSnippetEditorModel::setHotkey(int row, const QKeySequence& value)
{
    SETTER(snippetList[row]->data.hotkey, value.toString());
}

QKeySequence CodeSnippetEditorModel::getHotkey(int row) const
{
    GETTER(QKeySequence(snippetList[row]->data.hotkey), QKeySequence());
}

void CodeSnippetEditorModel::setData(const QList<CodeSnippetManager::CodeSnippet*>& snippets)
{
    beginResetModel();

    for (Snippet*& snippetPtr : snippetList)
        delete snippetPtr;

    snippetList.clear();

    for (CodeSnippetManager::CodeSnippet* snip : snippets)
        snippetList << new Snippet(snip);

    listModified = false;
    originalSnippetList = snippetList;

    endResetModel();
}

void CodeSnippetEditorModel::addSnippet(CodeSnippetManager::CodeSnippet* snippet)
{
    int row = snippetList.size();

    beginInsertRows(QModelIndex(), row, row);

    snippetList << new Snippet(snippet);
    listModified = true;

    endInsertRows();
}

void CodeSnippetEditorModel::deleteSnippet(int row)
{
    if (!isValidRowIndex(row))
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete snippetList[row];
    snippetList.removeAt(row);

    listModified = true;

    endRemoveRows();
}

QList<CodeSnippetManager::CodeSnippet*> CodeSnippetEditorModel::generateSnippets() const
{
    QList<CodeSnippetManager::CodeSnippet*> results;
    for (Snippet* snip: snippetList)
        results << new CodeSnippetManager::CodeSnippet(snip->data);

    return results;
}

QStringList CodeSnippetEditorModel::getSnippetNames() const
{
    QStringList names;
    for (Snippet* snip : snippetList)
        names << snip->data.name;

    return names;
}

void CodeSnippetEditorModel::validateNames()
{
    StrHash<QList<int>> counter;

    int row = 0;
    for (Snippet*& snip : snippetList)
    {
        snip->valid &= true;
        counter[snip->data.name] << row++;
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
    for (int i = 0; i < snippetList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool CodeSnippetEditorModel::isAllowedName(int rowToSkip, const QString& nameToValidate)
{
    QStringList names = getSnippetNames();
    names.removeAt(rowToSkip);
    return !names.contains(nameToValidate, Qt::CaseInsensitive);
}

bool CodeSnippetEditorModel::isAllowedHotkey(int rowToSkip, const QKeySequence& hotkeyToValidate)
{
    QList<QKeySequence> keys;
    for (Snippet*& snip : snippetList)
        keys << snip->data.hotkey;

    keys.removeAt(rowToSkip);
    return !keys.contains(hotkeyToValidate);
}

bool CodeSnippetEditorModel::isValidRowIndex(int row) const
{
    return (row >= 0 && row < snippetList.size());
}

int CodeSnippetEditorModel::moveUp(int row)
{
    if (row <= 0 || row >= snippetList.size())
        return row;

    snippetList.move(row, row - 1);
    return row - 1;
}

int CodeSnippetEditorModel::moveDown(int row)
{
    if (row < 0 || row + 1 >= snippetList.size())
        return row;

    snippetList.move(row, row + 1);
    return row + 1;
}

int CodeSnippetEditorModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return snippetList.size();
}

QVariant CodeSnippetEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRowIndex(index.row()))
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        Snippet* sn = snippetList[index.row()];
        return sn->data.name;
    }

    if (role == Qt::DecorationRole)
    {
        return snippetList[index.row()]->valid ? ICONS.CODE_SNIPPET : ICONS.CODE_SNIPPET_ERROR;
    }

    return QVariant();
}

void CodeSnippetEditorModel::emitDataChanged(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

CodeSnippetEditorModel::Snippet::Snippet()
{
}

CodeSnippetEditorModel::Snippet::Snippet(CodeSnippetManager::CodeSnippet* other)
{
    data = CodeSnippetManager::CodeSnippet(*other);
    originalName = data.name;
}
