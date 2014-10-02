#include "completermodel.h"
#include "iconmanager.h"
#include "common/unused.h"
#include "completerview.h"
#include <QIcon>
#include <QVariant>

CompleterModel::CompleterModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}


QModelIndex CompleterModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
        return QModelIndex(); // no childrens

    return createIndex(row, column);
}

QModelIndex CompleterModel::parent(const QModelIndex& child) const
{
    UNUSED(child);
    return QModelIndex();
}

int CompleterModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return tokens.size();
}

int CompleterModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return 1;
}

QVariant CompleterModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= tokens.size())
        return QVariant();

    ExpectedTokenPtr token = tokens[row];
    switch (role)
    {
        case Qt::DisplayRole:
        case VALUE:
            return token->value;
        case CONTEXT:
            return token->contextInfo;
        case LABEL:
            return token->label;
        case PREFIX:
            return token->prefix;
        case TYPE:
            return (int)token->type;
        case Qt::DecorationRole:
            return getIcon(token->type);
    }

    return QVariant();
}

void CompleterModel::setCompleterView(CompleterView* view)
{
    completerView = view;
}

void CompleterModel::setData(const QList<ExpectedTokenPtr>& data)
{
    clear();
    beginInsertRows(QModelIndex(), 0, data.size()-1);
    tokens = data;
    endInsertRows();
}

void CompleterModel::setFilter(const QString& filter)
{
    this->filter = filter;
    applyFilter();
}

QString CompleterModel::getFilter() const
{
    return filter;
}

void CompleterModel::applyFilter()
{
    bool empty = filter.isEmpty();
    QModelIndex idx;
    QString value;
    QString prefix;
    bool matched = empty;
    for (int i = 0; i < rowCount(QModelIndex()); i++)
    {
        if (!empty)
        {
            idx = index(i, 0, QModelIndex());
            value = idx.data(VALUE).toString();
            prefix = idx.data(PREFIX).toString();
            if (!prefix.isEmpty())
                value.prepend(prefix+".");

            matched = value.startsWith(filter, Qt::CaseInsensitive);
        }

        completerView->setRowHidden(i, !matched);
    }
}


void CompleterModel::clear()
{
    beginResetModel();
    tokens.clear();
    endResetModel();
}

ExpectedTokenPtr CompleterModel::getToken(int index) const
{
    if (index < 0 || index >= tokens.size())
        return ExpectedTokenPtr();

    return tokens[index];
}

QIcon CompleterModel::getIcon(ExpectedToken::Type type) const
{
    switch (type)
    {
        case ExpectedToken::COLUMN:
            return ICONS.COLUMN;
        case ExpectedToken::TABLE:
            return ICONS.TABLE;
        case ExpectedToken::INDEX:
            return ICONS.INDEX;
        case ExpectedToken::TRIGGER:
            return ICONS.TRIGGER;
        case ExpectedToken::VIEW:
            return ICONS.VIEW;
        case ExpectedToken::DATABASE:
            return ICONS.DATABASE;
        case ExpectedToken::OTHER:
            return ICONS.COMPLETER_OTHER;
        case ExpectedToken::KEYWORD:
            return ICONS.KEYWORD;
        case ExpectedToken::FUNCTION:
            return ICONS.FUNCTION;
        case ExpectedToken::OPERATOR:
            return ICONS.COMPLETER_OPERATOR;
        case ExpectedToken::STRING:
            return ICONS.COMPLETER_STRING;
        case ExpectedToken::NUMBER:
            return ICONS.COMPLETER_NUMBER;
        case ExpectedToken::BLOB:
            return ICONS.COMPLETER_BLOB;
        case ExpectedToken::COLLATION:
            return ICONS.CONSTRAINT_COLLATION;
        case ExpectedToken::PRAGMA:
            return ICONS.COMPLETER_PRAGMA;
        case ExpectedToken::NO_VALUE:
            return ICONS.COMPLETER_NO_VALUE;
    }

    return ICONS.COMPLETER_NO_VALUE;
}
