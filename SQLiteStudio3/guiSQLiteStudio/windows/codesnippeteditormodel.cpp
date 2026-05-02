#include "codesnippeteditormodel.h"
#include "common/strhash.h"
#include "iconmanager.h"
#include "common/global.h"
#include <QKeySequence>
#include <QListWidgetItem>
#include <QMimeData>

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

bool CodeSnippetEditorModel::isValid() const
{
    for (Snippet* snip : snippetList)
    {
        if (!snip->valid)
            return false;
    }
    return true;
}

void CodeSnippetEditorModel::setSnippets(const QList<CodeSnippetManager::CodeSnippet*>& snippets)
{
    beginResetModel();
    qDeleteAll(snippetList);
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

void CodeSnippetEditorModel::deleteSnippet(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    beginRemoveRows(QModelIndex(), idx.row(), idx.row());

    delete snippetList[idx.row()];
    snippetList.removeAt(idx.row());

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
                setData(index(cntRow), false, VALID);
        }
    }

    QModelIndex idx;
    for (int i = 0; i < snippetList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool CodeSnippetEditorModel::isAllowedName(const QModelIndex& idx, const QString& nameToValidate)
{
    QStringList names = getSnippetNames();
    names.removeAt(idx.row());
    return !names.contains(nameToValidate, Qt::CaseInsensitive);
}

bool CodeSnippetEditorModel::isAllowedHotkey(const QModelIndex& idx, const QKeySequence& hotkeyToValidate)
{
    QList<QKeySequence> keys;
    for (Snippet*& snip : snippetList)
        keys << snip->data.hotkey;

    keys.removeAt(idx.row());
    return !keys.contains(hotkeyToValidate);
}

void CodeSnippetEditorModel::setDragEnabled(bool value)
{
    dragEnabled = value;
}

QModelIndex CodeSnippetEditorModel::moveUp(const QModelIndex& idx)
{
    if (!idx.isValid() || idx.row() <= 0)
        return idx;

    const int sourceRow = idx.row();
    const int destinationChild = sourceRow - 1;

    beginMoveRows(QModelIndex(), sourceRow, sourceRow,
                  QModelIndex(), destinationChild);

    snippetList.move(sourceRow, sourceRow - 1);

    endMoveRows();
    return index(sourceRow - 1, 0);
}

QModelIndex CodeSnippetEditorModel::moveDown(const QModelIndex& idx)
{
    if (!idx.isValid() || idx.row() + 1 >= snippetList.size())
        return idx;

    const int sourceRow = idx.row();

    beginMoveRows(QModelIndex(), sourceRow, sourceRow,
                  QModelIndex(), sourceRow + 2);

    snippetList.move(sourceRow, sourceRow + 1);

    endMoveRows();
    return index(sourceRow + 1, 0);
}

int CodeSnippetEditorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return snippetList.size();
}

int CodeSnippetEditorModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant CodeSnippetEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.column() < 0 || index.column() > 1)
        return {};

    if (index.row() < 0 || index.row() >= snippetList.size())
        return {};

    const Snippet* sn = snippetList[index.row()];
    switch (index.column())
    {
        case 0:
        {
            switch (role)
            {
                case Qt::DisplayRole:
                    return sn->data.name;
                case MODIFIED:
                    return sn->modified;
                case VALID:
                    return sn->valid;
                case CODE:
                    return sn->data.code;
                case HOTKEY:
                    return sn->data.hotkey;
                case Qt::DecorationRole:
                    return sn->valid ? ICONS.CODE_SNIPPET : ICONS.CODE_SNIPPET_ERROR;
            }
        }
        case 1:
        {
            if (role == Qt::DisplayRole)
                return sn->data.hotkey;

            break;
        }
    }

    if (role == Qt::ToolTipRole)
    {
        static_qstring(rowTpl, "<tr><td align='right'>%1</td><td><b>%2</b></td></tr>");
        return "<table>" +
               rowTpl.arg(tr("Name:"), sn->data.name) +
               rowTpl.arg(tr("Hotkey:"), sn->data.hotkey) +
               "</table>";
    }

    return QVariant();
}

bool CodeSnippetEditorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    if (index.column() != 0)
        return false;

    if (index.row() < 0 || index.row() >= snippetList.size())
        return false;

    Snippet* sn = snippetList[index.row()];
    switch (role)
    {
        case Qt::DisplayRole:
            sn->data.name = value.toString();
            break;
        case MODIFIED:
            sn->modified = value.toBool();
            break;
        case VALID:
            sn->valid = value.toBool();
            break;
        case CODE:
            sn->data.code = value.toString();
            break;
        case HOTKEY:
            sn->data.hotkey = value.toString();
            break;
        defaut:
            return true;
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags CodeSnippetEditorModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (dragEnabled && index.isValid())
        return defaultFlags | Qt::ItemIsDragEnabled;

    return defaultFlags;
}

QStringList CodeSnippetEditorModel::mimeTypes() const
{
    if (!dragEnabled)
        return {};

    return {"text/plain", MIMETYPE};
}

QMimeData* CodeSnippetEditorModel::mimeData(const QModelIndexList& indexes) const
{
    if (!dragEnabled)
        return nullptr;

    QMimeData *mimeData = new QMimeData();
    if (indexes.isEmpty())
        return mimeData;

    QString text = indexes.first().data(CODE).toString();
    mimeData->setText(text);
    mimeData->setData(MIMETYPE, text.toUtf8());

    return mimeData;
}

CodeSnippetEditorModel::Snippet::Snippet()
{
}

CodeSnippetEditorModel::Snippet::Snippet(CodeSnippetManager::CodeSnippet* other)
{
    data = CodeSnippetManager::CodeSnippet(*other);
    originalName = data.name;
}
