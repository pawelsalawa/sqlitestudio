#include "tableconstraintsmodel.h"
#include "iconmanager.h"
#include "common/utils_sql.h"
#include "common/unused.h"
#include <QDebug>
#include <QMimeData>

TableConstraintsModel::TableConstraintsModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int TableConstraintsModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    if (createTable.isNull())
        return 0;

    return createTable->constraints.size();
}

int TableConstraintsModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return 3;
}

QVariant TableConstraintsModel::data(const QModelIndex& index, int role) const
{
    if (createTable.isNull())
        return QVariant();

    SqliteCreateTable::Constraint* constr = createTable->constraints[index.row()];
    switch (getColumn(index.column()))
    {
        case Columns::TYPE:
        {
            if (role == Qt::DisplayRole)
                return getTypeLabel(constr->type);

            if (role == Qt::DecorationRole)
                return getTypeIcon(constr->type);

            break;
        }
        case Columns::NAME:
        {
            if (role == Qt::DisplayRole)
                return stripObjName(constr->name, createTable->dialect);

            break;
        }
        case Columns::DETAILS:
        {
            if (role == Qt::DisplayRole)
                return getDetails(constr);

            break;
        }
    }
    return QVariant();
}

QVariant TableConstraintsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (getColumn(section))
    {
        case TableConstraintsModel::Columns::TYPE:
            return tr("Type", "table constraints");
        case TableConstraintsModel::Columns::DETAILS:
            return tr("Details", "table constraints");
        case TableConstraintsModel::Columns::NAME:
            return tr("Name", "table constraints");
    }
    return QVariant();
}

Qt::DropActions TableConstraintsModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions TableConstraintsModel::supportedDragActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}

QStringList TableConstraintsModel::mimeTypes() const
{
    return {mimeType};
}

QMimeData* TableConstraintsModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.size() < 1)
        return nullptr;

    QModelIndex idx = indexes.first();

    QMimeData *data = new QMimeData();

    QByteArray output;
    QDataStream stream(&output, QIODevice::WriteOnly);

    stream << idx.row();
    data->setData(mimeType, output);

    return data;
}

bool TableConstraintsModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    UNUSED(action);
    UNUSED(row);
    UNUSED(column);
    UNUSED(parent);

    if (!data)
        return false;

    if (!data->hasFormat(mimeType))
        return false;

    return true;
}

bool TableConstraintsModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    UNUSED(column);

    if (action == Qt::IgnoreAction)
        return true;

    if (!data)
        return false;

    if (!data->hasFormat(mimeType))
        return false;

    if (action != Qt::MoveAction)
        return false;

    if (row < 0)
    {
        if (!parent.isValid() && !createTable.isNull())
            row = createTable->constraints.size();
        else
            row = parent.row();
    }

    if (row < 0)
        return false;

    QByteArray byteData = data->data(mimeType);
    QDataStream stream(&byteData, QIODevice::ReadOnly);
    int oldRow;
    stream >> oldRow;

    moveConstraintColumnTo(oldRow, row);
    return true;
}

Qt::ItemFlags TableConstraintsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return defFlags|Qt::ItemIsDropEnabled;

    return defFlags|Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled;
}

bool TableConstraintsModel::isModified() const
{
    return modified;
}

void TableConstraintsModel::setCreateTable(SqliteCreateTable* value)
{
    beginResetModel();
    createTable = value;
    endResetModel();
    modified = false;
    emit modifiyStateChanged();
}

SqliteCreateTable::Constraint* TableConstraintsModel::getConstraint(int constrIdx) const
{
    if (createTable.isNull())
        return nullptr;

    return createTable->constraints[constrIdx];
}

void TableConstraintsModel::replaceConstraint(int constrIdx, SqliteCreateTable::Constraint* constr)
{
    if (createTable.isNull())
        return;

    delete createTable->constraints[constrIdx];
    createTable->constraints[constrIdx] = constr;
    constr->setParent(createTable);
    modified = true;

    emit modifiyStateChanged();
    emit dataChanged(createIndex(constrIdx, 0), createIndex(constrIdx, columnCount()-1));
}

void TableConstraintsModel::constraintModified(int constrIdx)
{
    modified = true;

    emit modifiyStateChanged();
    emit dataChanged(createIndex(constrIdx, 0), createIndex(constrIdx, columnCount()-1));
}

void TableConstraintsModel::insertConstraint(int constrIdx, SqliteCreateTable::Constraint* constr)
{
    if (createTable.isNull())
        return;

    beginInsertRows(QModelIndex(), constrIdx, constrIdx);
    createTable->constraints.insert(constrIdx, constr);
    constr->setParent(createTable);
    endInsertRows();

    modified = true;
    emit modifiyStateChanged();
}

void TableConstraintsModel::appendConstraint(SqliteCreateTable::Constraint* constr)
{
    if (createTable.isNull())
        return;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    createTable->constraints.append(constr);
    constr->setParent(createTable);
    endInsertRows();

    modified = true;
    emit modifiyStateChanged();
}

void TableConstraintsModel::delConstraint(int constrIdx)
{
    if (createTable.isNull())
        return;

    beginRemoveRows(QModelIndex(), constrIdx, constrIdx);
    delete createTable->constraints[constrIdx];
    createTable->constraints.removeAt(constrIdx);
    endRemoveRows();

    modified = true;
    emit modifiyStateChanged();
}

void TableConstraintsModel::moveConstraintUp(int constrIdx)
{
    moveConstraintColumnTo(constrIdx, constrIdx-1);
}

void TableConstraintsModel::moveConstraintDown(int constrIdx)
{
    moveConstraintColumnTo(constrIdx, constrIdx+1);
}

void TableConstraintsModel::moveConstraintColumnTo(int constrIdx, int newIdx)
{
    if (createTable.isNull())
        return;

    if (newIdx == constrIdx)
        return;

    if (newIdx == constrIdx+1)
    {
        // See TableStructureModel::moveColumnTo() for details above code below.
        int tmpIdx = newIdx;
        newIdx = constrIdx;
        constrIdx = tmpIdx;
    }

    beginMoveRows(QModelIndex(), constrIdx, constrIdx, QModelIndex(), newIdx);
    if (newIdx >= createTable->constraints.size())
    {
        SqliteCreateTable::Constraint* constr = createTable->constraints.takeAt(constrIdx);
        createTable->constraints.append(constr);
    }
    else
        createTable->constraints.move(constrIdx, newIdx);

    endMoveRows();

    modified = true;
    emit modifiyStateChanged();
    emit constraintOrderChanged();
}

TableConstraintsModel::Columns TableConstraintsModel::getColumn(int idx) const
{
    return static_cast<Columns>(idx);
}

QString TableConstraintsModel::getTypeLabel(SqliteCreateTable::Constraint::Type type) const
{
    switch (type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return "PRIMARY KEY";
        case SqliteCreateTable::Constraint::UNIQUE:
            return "UNIQUE";
        case SqliteCreateTable::Constraint::CHECK:
            return "CHECK";
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return "FOREIGN KEY";
        case SqliteCreateTable::Constraint::NAME_ONLY:
            return QString::null;
    }
    return QString::null;
}

QIcon TableConstraintsModel::getTypeIcon(SqliteCreateTable::Constraint::Type type) const
{
    switch (type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return ICONS.CONSTRAINT_PRIMARY_KEY;
        case SqliteCreateTable::Constraint::UNIQUE:
            return ICONS.CONSTRAINT_UNIQUE;
        case SqliteCreateTable::Constraint::CHECK:
            return ICONS.CONSTRAINT_CHECK;
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return ICONS.CONSTRAINT_FOREIGN_KEY;
        case SqliteCreateTable::Constraint::NAME_ONLY:
            return QIcon();
    }
    return QIcon();
}

QString TableConstraintsModel::getDetails(SqliteCreateTable::Constraint* constr) const
{
    switch (constr->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return getPkDetails(constr);
        case SqliteCreateTable::Constraint::UNIQUE:
            return getUniqueDetails(constr);
        case SqliteCreateTable::Constraint::CHECK:
            return getCheckDetails(constr);
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return getFkDetails(constr);
        case SqliteCreateTable::Constraint::NAME_ONLY:
            return QString::null;
    }
    return QString::null;
}

QString TableConstraintsModel::getPkDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString TableConstraintsModel::getUniqueDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "UNIQUE", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString TableConstraintsModel::getCheckDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "CHECK", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString TableConstraintsModel::getFkDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString TableConstraintsModel::getConstrDetails(SqliteCreateTable::Constraint* constr, int tokenOffset) const
{
    TokenList tokens = constr->tokens.mid(tokenOffset);
    tokens.trimLeft();
    return tokens.detokenize();
}

void TableConstraintsModel::columnRenamed(SqliteCreateTable::Constraint* constr, const QString& oldColumn, const QString& newColumn)
{
    foreach (SqliteIndexedColumn* idxCol, constr->indexedColumns)
    {
        if (idxCol->name.compare(oldColumn, Qt::CaseInsensitive) == 0)
        {
            idxCol->name = newColumn;
            modified = true;
        }
    }

    emit modifiyStateChanged();
}

bool TableConstraintsModel::handleColumnDeleted(SqliteCreateTable::Constraint* constr, const QString& column)
{
    switch (constr->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
        case SqliteCreateTable::Constraint::UNIQUE:
        {
            QMutableListIterator<SqliteIndexedColumn*> it(constr->indexedColumns);
            SqliteIndexedColumn* idxCol;
            while (it.hasNext())
            {
                idxCol = it.next();
                if (idxCol->name.compare(column, Qt::CaseInsensitive) == 0)
                {
                    it.remove();
                    delete idxCol;
                    modified = true;
                }
            }

            emit modifiyStateChanged();
            return constr->indexedColumns.count() > 0;
        }
        case SqliteCreateTable::Constraint::CHECK:
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
        case SqliteCreateTable::Constraint::NAME_ONLY:
            break;
    }
    return true;
}

void TableConstraintsModel::columnModified(const QString& oldColumn, SqliteCreateTable::Column* newColumn)
{
    if (newColumn->name == oldColumn)
        return;

    int idx = 0;
    foreach (SqliteCreateTable::Constraint* constr, createTable->constraints)
    {
        if (constr->doesAffectColumn(oldColumn))
        {
            columnRenamed(constr, oldColumn, newColumn->name);
            constr->rebuildTokens();
            emit dataChanged(createIndex(idx, 0), createIndex(idx, columnCount()-1));
        }

        idx++;
    }
}

void TableConstraintsModel::columnDeleted(const QString& column)
{
    QList<int> toDelete;
    int idx = 0;
    foreach (SqliteCreateTable::Constraint* constr, createTable->constraints)
    {
        if (constr->doesAffectColumn(column))
        {
            if (handleColumnDeleted(constr, column))
            {
                constr->rebuildTokens();
                emit dataChanged(createIndex(idx, 0), createIndex(idx, columnCount()-1));
            }
            else
                toDelete << idx;
        }

        idx++;
    }

    foreach (int idx, toDelete)
        delConstraint(idx);
}
