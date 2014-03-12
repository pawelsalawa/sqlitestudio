#include "constrainttabmodel.h"
#include "common/unused.h"
#include "iconmanager.h"
#include "common/utils_sql.h"
#include <QDebug>

ConstraintTabModel::ConstraintTabModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int ConstraintTabModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    if (createTable.isNull())
        return 0;

    int cnt = 0;
    foreach (SqliteCreateTable::Column* col, createTable->columns)
        cnt += col->constraints.size();

    cnt += createTable->constraints.size();
    return cnt;
}

int ConstraintTabModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return 4;
}

QVariant ConstraintTabModel::data(const QModelIndex& index, int role) const
{
    if (createTable.isNull())
        return QVariant();

    int constrIdx = index.row();
    int currIdx = -1;
    foreach (SqliteCreateTable::Column* column, createTable->columns)
    {
        foreach (SqliteCreateTable::Column::Constraint* constr, column->constraints)
        {
            currIdx++;

            if (currIdx == constrIdx)
                return data(constr, index.column(), role);
        }
    }

    foreach (SqliteCreateTable::Constraint* constr, createTable->constraints)
    {
        currIdx++;

        if (currIdx == constrIdx)
            return data(constr, index.column(), role);
    }

    return QVariant();
}

QVariant ConstraintTabModel::data(SqliteCreateTable::Constraint* constr, int column, int role) const
{
    switch (getColumn(column))
    {
        case ConstraintTabModel::Columns::SCOPE:
        {
            if (role == Qt::DisplayRole)
                return tr("Table", "table constraints");

            break;
        }
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

QVariant ConstraintTabModel::data(SqliteCreateTable::Column::Constraint* constr, int column, int role) const
{
    switch (getColumn(column))
    {
        case ConstraintTabModel::Columns::SCOPE:
        {
            if (role == Qt::DisplayRole)
            {
                QString colName = dynamic_cast<SqliteCreateTable::Column*>(constr->parentStatement())->name;
                return tr("Column (%1)", "table constraints").arg(colName);
            }

            break;
        }
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

QVariant ConstraintTabModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (getColumn(section))
    {
        case ConstraintTabModel::Columns::SCOPE:
            return tr("Scope", "table constraints");
        case Columns::TYPE:
            return tr("Type", "table constraints");
        case Columns::DETAILS:
            return tr("Details", "table constraints");
        case Columns::NAME:
            return tr("Name", "table constraints");
    }
    return QVariant();
}

void ConstraintTabModel::setCreateTable(const QPointer<SqliteCreateTable>& value)
{
    beginResetModel();
    createTable = value;
    endResetModel();
}

ConstraintTabModel::Columns ConstraintTabModel::getColumn(int idx) const
{
    return static_cast<Columns>(idx);
}

QString ConstraintTabModel::getTypeLabel(SqliteCreateTable::Constraint::Type type) const
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

QString ConstraintTabModel::getTypeLabel(SqliteCreateTable::Column::Constraint::Type type) const
{
    switch (type)
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

QIcon ConstraintTabModel::getTypeIcon(SqliteCreateTable::Constraint::Type type) const
{
    switch (type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return ICON("pk");
        case SqliteCreateTable::Constraint::UNIQUE:
            return ICON("unique");
        case SqliteCreateTable::Constraint::CHECK:
            return ICON("check");
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return ICON("fk");
        case SqliteCreateTable::Constraint::NAME_ONLY:
            return QIcon();
    }
    return QIcon();
}

QIcon ConstraintTabModel::getTypeIcon(SqliteCreateTable::Column::Constraint::Type type) const
{
    switch (type)
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

QString ConstraintTabModel::getDetails(SqliteCreateTable::Constraint* constr) const
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

QString ConstraintTabModel::getDetails(SqliteCreateTable::Column::Constraint* constr) const
{
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

QString ConstraintTabModel::getPkDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getUniqueDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "UNIQUE", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getCheckDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "CHECK", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getFkDetails(SqliteCreateTable::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getPkDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "KEY", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getUniqueDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "UNIQUE", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getCheckDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "CHECK", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getFkDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "REFERENCES", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx);
}

QString ConstraintTabModel::getNotNullDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "NULL", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getCollateDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "COLLATE", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getDefaultDetails(SqliteCreateTable::Column::Constraint* constr) const
{
    int idx = constr->tokens.indexOf(Token::KEYWORD, "DEFAULT", Qt::CaseInsensitive);
    return getConstrDetails(constr, idx + 1);
}

QString ConstraintTabModel::getConstrDetails(SqliteCreateTable::Constraint* constr, int tokenOffset) const
{
    return getConstrDetails(constr->tokens, tokenOffset);
}

QString ConstraintTabModel::getConstrDetails(SqliteCreateTable::Column::Constraint* constr, int tokenOffset) const
{
    return getConstrDetails(constr->tokens, tokenOffset);
}

QString ConstraintTabModel::getConstrDetails(const TokenList& constrTokens, int tokenOffset) const
{
    TokenList tokens = constrTokens.mid(tokenOffset);
    tokens.trimLeft();
    return tokens.detokenize();
}

void ConstraintTabModel::updateModel()
{
    beginResetModel();
    endResetModel();
}
