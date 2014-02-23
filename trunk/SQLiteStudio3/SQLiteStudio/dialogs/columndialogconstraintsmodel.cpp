#include "columndialogconstraintsmodel.h"
#include "unused.h"
#include "iconmanager.h"

ColumnDialogConstraintsModel::ColumnDialogConstraintsModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void ColumnDialogConstraintsModel::setColumn(SqliteCreateTable::Column* value)
{
    beginResetModel();
    column = value;
    endResetModel();
}

SqliteCreateTable::Column::Constraint* ColumnDialogConstraintsModel::getConstraint(int constrIdx) const
{
    if (column.isNull())
        return nullptr;

    return column->constraints[constrIdx];
}

void ColumnDialogConstraintsModel::replaceConstraint(int constrIdx, SqliteCreateTable::Column::Constraint* constr)
{
    if (column.isNull())
        return;

    delete column->constraints[constrIdx];
    column->constraints[constrIdx] = constr;
    constr->setParent(column);

    emit constraintsChanged();
}

void ColumnDialogConstraintsModel::insertConstraint(int constrIdx, SqliteCreateTable::Column::Constraint* constr)
{
    if (column.isNull())
        return;

    beginInsertRows(QModelIndex(), constrIdx, constrIdx);
    column->constraints.insert(constrIdx, constr);
    constr->setParent(column);
    endInsertRows();

    emit constraintsChanged();
}

void ColumnDialogConstraintsModel::appendConstraint(SqliteCreateTable::Column::Constraint* constr)
{
    if (column.isNull())
        return;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    column->constraints.append(constr);
    constr->setParent(column);
    endInsertRows();

    emit constraintsChanged();
}

void ColumnDialogConstraintsModel::delConstraint(int constrIdx)
{
    if (column.isNull())
        return;

    beginRemoveRows(QModelIndex(), constrIdx, constrIdx);
    delete column->constraints[constrIdx];
    column->constraints.removeAt(constrIdx);
    endRemoveRows();

    emit constraintsChanged();
}

void ColumnDialogConstraintsModel::delConstraint(SqliteCreateTable::Column::Constraint* constr)
{
    if (column.isNull())
        return;

    int constrIdx = column->constraints.indexOf(constr);
    if (constrIdx < -1)
        return;

    delConstraint(constrIdx);
}

void ColumnDialogConstraintsModel::moveConstraintUp(int constrIdx)
{
    moveConstraintColumnTo(constrIdx, constrIdx-1);
}

void ColumnDialogConstraintsModel::moveConstraintDown(int constrIdx)
{
    moveConstraintColumnTo(constrIdx, constrIdx+1);
}

void ColumnDialogConstraintsModel::moveConstraintColumnTo(int constrIdx, int newIdx)
{
    if (column.isNull())
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
    if (newIdx >= column->constraints.size())
    {
        SqliteCreateTable::Column::Constraint* constr = column->constraints.takeAt(constrIdx);
        column->constraints.append(constr);
    }
    else
        column->constraints.move(constrIdx, newIdx);

    endMoveRows();

    emit constraintsChanged();
}

ColumnDialogConstraintsModel::Column ColumnDialogConstraintsModel::getColumn(int colIdx) const
{
    return static_cast<Column>(colIdx);
}

QIcon ColumnDialogConstraintsModel::getIcon(int rowIdx) const
{
    SqliteCreateTable::Column::Constraint* constr = column->constraints[rowIdx];
    switch (constr->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return ICON("pk");
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return ICON("not_null");
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return ICON("unique");
        case SqliteCreateTable::Column::Constraint::CHECK:
            return ICON("check");
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return ICON("default");
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return ICON("collation");
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return ICON("fk");
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return QIcon();
}

QString ColumnDialogConstraintsModel::getName(int rowIdx) const
{
    SqliteCreateTable::Column::Constraint* constr = column->constraints[rowIdx];
    return constr->name;
}

QString ColumnDialogConstraintsModel::getType(int rowIdx) const
{
    SqliteCreateTable::Column::Constraint* constr = column->constraints[rowIdx];
    switch (constr->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return "PRIMARY KEY";
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return "NOT NULL";
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return "UNIQUE";
        case SqliteCreateTable::Column::Constraint::CHECK:
            return "CHECK";
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return "DEFAULT";
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return "COLLATE";
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return "FOREIGN KEY";
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return QString::null;
}

QString ColumnDialogConstraintsModel::getDetails(int rowIdx) const
{
    SqliteCreateTable::Column::Constraint* constr = column->constraints[rowIdx];
    switch (constr->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return getPkDetails(constr);
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return getNotNullDetails(constr);
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return getUniqueDetails(constr);
        case SqliteCreateTable::Column::Constraint::CHECK:
            return getCheckDetails(constr);
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return getDefaultDetails(constr);
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return getCollateDetails(constr);
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return getFkDetails(constr);
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return QString::null;
}

QString ColumnDialogConstraintsModel::getPkDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getNotNullDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "NULL", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getUniqueDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "UNIQUE", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getCheckDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "CHECK", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getDefaultDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "DEFAULT", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getCollateDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "COLLATE", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getFkDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ColumnDialogConstraintsModel::getConstrDetails(SqliteCreateTable::Column::Constraint* constr, int tokenOffset) const
{
    TokenList tokens = constr->tokens.mid(tokenOffset + 1);
    tokens.trimLeft();
    return tokens.detokenize();
}

int ColumnDialogConstraintsModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    if (column.isNull())
        return 0;

    return column->constraints.size();
}

int ColumnDialogConstraintsModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return 3;
}

QVariant ColumnDialogConstraintsModel::data(const QModelIndex& index, int role) const
{
    if (column.isNull())
        return QVariant();

    switch (getColumn(index.column()))
    {
        case Column::TYPE:
        {
            if (role == Qt::DecorationRole)
                return getIcon(index.row());

            if (role == Qt::DisplayRole)
                return getType(index.row());

            break;
        }
        case Column::NAME:
        {
            if (role == Qt::DisplayRole)
                return getName(index.row());

            break;
        }
        case Column::DETAILS:
            if (role == Qt::DisplayRole)
                return getDetails(index.row());

            break;
    }
    return QVariant();
}

QVariant ColumnDialogConstraintsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (getColumn(section))
    {
        case Column::TYPE:
            return tr("Type", "column dialog constraints");
        case Column::NAME:
            return tr("Name", "column dialog constraints");
        case Column::DETAILS:
            return tr("Details", "column dialog constraints");
    }
    return QVariant();
}
