#include "sqlqueryitem.h"
#include "common/utils_sql.h"
#include "sqlquerymodel.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include "sqlqueryview.h"
#include "style.h"
#include <QDate>
#include <QDebug>
#include <QApplication>
#include <QStyle>

SqlQueryItem::SqlQueryItem(QObject *parent) :
    QObject(parent)
{
    setUncommitted(false);
    setCommittingError(false);
    setRowId(RowId());
    setColumn(nullptr);
}

SqlQueryItem::SqlQueryItem(const SqlQueryItem &item)
    : QObject(item.QObject::parent()), QStandardItem(item)
{
}

QStandardItem *SqlQueryItem::clone() const
{
    return new SqlQueryItem(*this);
}

RowId SqlQueryItem::getRowId() const
{
    return QStandardItem::data(DataRole::ROWID).toHash();
}

void SqlQueryItem::setRowId(const RowId& rowId)
{
    QStandardItem::setData(rowId, DataRole::ROWID);
}

bool SqlQueryItem::isUncommitted() const
{
    return QStandardItem::data(DataRole::UNCOMMITTED).toBool();
}

void SqlQueryItem::setUncommitted(bool uncommitted)
{
    QStandardItem::setData(QVariant(uncommitted), DataRole::UNCOMMITTED);
    if (!uncommitted)
    {
        clearOldValue();
        setCommittingError(false);
    }
}

void SqlQueryItem::rollback()
{
    setValue(getOldValue(), true);
    setUncommitted(false);
    setDeletedRow(false);
}

bool SqlQueryItem::isCommittingError() const
{
    return QStandardItem::data(DataRole::COMMITTING_ERROR).toBool();
}

void SqlQueryItem::setCommittingError(bool isError)
{
    QStandardItem::setData(QVariant(isError), DataRole::COMMITTING_ERROR);
    if (!isError)
        setCommittingErrorMessage(QString());
}

void SqlQueryItem::setCommittingError(bool isError, const QString& msg)
{
    setCommittingErrorMessage(msg);
    setCommittingError(isError);
}

QString SqlQueryItem::getCommittingErrorMessage() const
{
    return QStandardItem::data(DataRole::COMMITTING_ERROR_MESSAGE).toString();
}

void SqlQueryItem::setCommittingErrorMessage(const QString& value)
{
    QStandardItem::setData(QVariant(value), DataRole::COMMITTING_ERROR_MESSAGE);
}

bool SqlQueryItem::isNewRow() const
{
    return QStandardItem::data(DataRole::NEW_ROW).toBool();
}

void SqlQueryItem::setNewRow(bool isNew)
{
    QStandardItem::setData(QVariant(isNew), DataRole::NEW_ROW);
}

bool SqlQueryItem::isUntouched() const
{
    return QStandardItem::data(DataRole::UNTOUCHED).toBool();
}

void SqlQueryItem::setUntouched(bool isUntouched)
{
    QStandardItem::setData(QVariant(isUntouched), DataRole::UNTOUCHED);
}

bool SqlQueryItem::isDeletedRow() const
{
    return QStandardItem::data(DataRole::DELETED).toBool();
}

void SqlQueryItem::setDeletedRow(bool isDeleted)
{
    if (isDeleted && !getOldValue().isValid())
        rememberOldValue();

    QStandardItem::setData(QVariant(isDeleted), DataRole::DELETED);
}

QVariant SqlQueryItem::getValue() const
{
    return QStandardItem::data(DataRole::VALUE);
}

void SqlQueryItem::setValue(const QVariant &value, bool loadedFromDb)
{
    if (!valueSettingLock.tryLock())
    {
        // Triggered recursively by catching "itemChanged" event,
        // that was caused by the QStandardItem::setData below.
        return;
    }

    QVariant newValue = adjustVariantType(value);
    QVariant origValue = getValue();

    // It's modified when:
    // - original and new value is different (value or NULL status), while it's not loading from DB
    // - this item was already marked as uncommitted
    bool modified = (
                        (
                            newValue != origValue ||
                            isNull(origValue) != isNull(newValue) ||
                            newValue.typeId() != origValue.typeId()
                        ) &&
                        !loadedFromDb
                    ) ||
                    isUncommitted();

    if (modified && !getOldValue().isValid())
        rememberOldValue();

    // This is a workaround for an issue in Qt, that uses operator== to compare values in QStandardItem::setData().
    // If the old value is null and the new value is empty, then operator == returns true, which is a lie.
    // We need to trick QStandardItem to force updating the value. We feed it with any non-empty value,
    // then we can set whatever value we want and it should be updated (or not, if it's truly the same value).
    QStandardItem::setData("x", DataRole::VALUE);

    QStandardItem::setData(newValue, DataRole::VALUE);
    setUncommitted(modified);

    if (modified && getModel())
    {
        setUntouched(false);
        getModel()->itemValueEdited(this);
    }

    valueSettingLock.unlock();
}

QVariant SqlQueryItem::getOldValue() const
{
    return QStandardItem::data(DataRole::OLD_VALUE);
}

void SqlQueryItem::setOldValue(const QVariant& value)
{
    QStandardItem::setData(value, DataRole::OLD_VALUE);
}

QVariant SqlQueryItem::adjustVariantType(const QVariant& value)
{
    QVariant newValue;
    bool ok;
    newValue = value.toLongLong(&ok);
    if (ok)
    {
        ok = (value.toString() == newValue.toString());
        if (ok)
            return newValue;
    }

    newValue = value.toDouble(&ok);
    if (ok)
    {
        ok = (value.toString() == newValue.toString());
        if (ok)
            return newValue;
    }

    return value;
}

QString SqlQueryItem::getToolTip() const
{
    if (!index().isValid())
        return QString();

    SqlQueryModelColumn* col = getColumn();
    if (!col)
        return QString(); // happens when simple execution method was performed

    return formatToolTip(col, true, getRowId(), isCommittingError(), getCommittingErrorMessage());
}

QString SqlQueryItem::formatToolTip(SqlQueryModelColumn* col, const QString& footer)
{
    return formatToolTip(col, false, RowId(), false, QString(), footer);
}

QString SqlQueryItem::formatToolTip(SqlQueryModelColumn* col, bool withRowId, RowId rowId, bool withCommittingError, const QString& committingErrorMsg, const QString& footer)
{
    static const QString tableTmp = "<div><table>%1</table></div>";
    static const QString rowTmp = R"(
        <tr>
            <td colspan=2 style="white-space: pre">%1</td>
            <td style="text-align: right"><b>%2</b></td>
        </tr>)";
    static const QString hdrRowTmp = R"(
        <tr>
            <th width=16><img src="%1" width="16" height="16"/></th>
            <th colspan=2 style="align: center">%2 %3</th>
        </tr>)";
    static const QString constrRowTmp = R"(
        <tr>
            <td width=16 style="text-align: center"><img src="%1" width="16" height="16"/></td>
            <td style="white-space: pre"><b>%2</b></td>
            <td><code>%3</code></td>
        </tr>)";
    static const QString emptyRow = "<tr><td colspan=3></td></tr>";
    static const QString topErrorRowTmp = R"(
        <tr>
            <td width=16><img src="%1" width="16" height="16"/></td>
            <td style="white-space: pre"><b>%2</b></td>
            <td>%3</td>
        </tr>)";
    static const QString footerRowTmp = R"(
        <tr>
            <td colspan=3>%1</td>
        </tr>)";

    QStringList rows;
    if (withCommittingError)
    {
        rows << topErrorRowTmp.arg(ICONS.STATUS_WARNING.getPath(), tr("Committing error:", "data view tooltip"), committingErrorMsg);
        rows << emptyRow;
    }

    rows << hdrRowTmp.arg(ICONS.COLUMN.getPath(), tr("Column:", "data view tooltip"), col->column);
    rows << rowTmp.arg(tr("Data type:", "data view"), col->dataType.toString());
    if (!col->table.isNull())
    {
        rows << rowTmp.arg(tr("Table:", "data view tooltip"), col->table);

        if (withRowId)
        {
            QString rowIdStr;
            if (rowId.size() == 0)
            {
                rowIdStr = "-";
            }
            else if (rowId.size() == 1)
            {
                rowIdStr = rowId.values().constFirst().toString();
            }
            else
            {
                QStringList values;
                QString rowIdValue;
                QHashIterator<QString,QVariant> it(rowId);
                while (it.hasNext())
                {
                    it.next();
                    rowIdValue = it.value().toString();
                    if (rowIdValue.length() > 30)
                        rowIdValue = rowIdValue.left(27) + "...";

                    values << it.key() + "=" + rowIdValue;
                }
                rowIdStr = "[" + values.join(", ") + "]";
            }
            rows << rowTmp.arg("ROWID:", rowIdStr);
        }
    }

    if (!col->getConstraints().isEmpty())
    {
        rows << emptyRow;
        rows << hdrRowTmp.arg(ICONS.COLUMN_CONSTRAINT.getPath(), tr("Constraints:", "data view tooltip"), "");
        for (auto&& constr : col->getConstraints())
            rows << constrRowTmp.arg(constr->getIcon()->toUrl(), constr->getTypeString(), constr->getDetails());
    }

    if (!footer.isEmpty())
    {
        rows += footerRowTmp.arg("<hr>");
        rows += footerRowTmp.arg(footer);
    }
    return tableTmp.arg(rows.join(""));
}

void SqlQueryItem::rememberOldValue()
{
    setOldValue(getValue());
}

void SqlQueryItem::clearOldValue()
{
    setOldValue(QVariant());
}

QString SqlQueryItem::getNullDisplayString() const
{
    if (isUntouched())
    {
        SqlQueryModelColumn* col = getColumn();
        if (!col)
            return "DEFAULT";

        if (col->isDefault())
            return "DEFAULT";

        if (col->isGenerated())
            return "GENERATED";

        if (col->isPk() && col->isAutoIncr())
            return "#";
    }

    return "NULL";
}

SqlQueryModelColumn* SqlQueryItem::getColumn() const
{
    return QStandardItem::data(DataRole::COLUMN).value<SqlQueryModelColumn*>();
}

void SqlQueryItem::setColumn(SqlQueryModelColumn* column)
{
    QStandardItem::setData(QVariant::fromValue(column), DataRole::COLUMN);
}

SqlQueryModel *SqlQueryItem::getModel() const
{
    if (!model())
        return nullptr;

    return dynamic_cast<SqlQueryModel*>(model());
}

void SqlQueryItem::setData(const QVariant &value, int role)
{
    switch (role)
    {
        case Qt::EditRole:
        {
            // -1 column is used by Qt for header items (ie. when setHeaderData() is called)
            // and we want this to mean that the value was loaded from db, because it forces
            // the value to be interpreted as not modified.
            setValue(value, (column() == -1));
            return;
        }
    }

    QStandardItem::setData(value, role);
}

QVariant SqlQueryItem::data(int role) const
{
    switch (role)
    {
        case Qt::EditRole:
        {
            if (isDeletedRow())
                return QVariant();

            return getValue();
        }
        case Qt::DisplayRole:
        {
            if (isDeletedRow())
                return "";

            QVariant value = getValue();
            if (isNull(value))
                return getNullDisplayString();

            if (value.metaType() == QMetaType::fromType<QString>())
            {
                QString str = value.toString();
                return str.length() > DISPLAY_LEN_LIMIT ? QVariant(str.left(DISPLAY_LEN_LIMIT) + "...") : value;
            }

            if (value.metaType() == QMetaType::fromType<QByteArray>())
            {
                QByteArray bytes = value.toByteArray();
                return bytes.size() > DISPLAY_LEN_LIMIT ? QVariant(bytes.left(DISPLAY_LEN_LIMIT) + "...") : value;
            }

            return value;
        }
        case Qt::ForegroundRole:
        {
            QVariant value = getValue();
            if (isNull(value))
                return Cfg::getSyntaxCommentFormat().foreground().color();

            break;
        }
        case Qt::BackgroundRole:
        {
            if (isDeletedRow())
                return QApplication::style()->standardPalette().dark();

            break;
        }
        case Qt::TextAlignmentRole:
        {
            QVariant value = getValue();
            if (isNull(value) || isDeletedRow())
                return Qt::AlignCenter;

            break;
        }
        case Qt::FontRole:
        {
            QFont font = CFG_UI.Fonts.DataView.get();

            QVariant value = getValue();
            if (isNull(value) || isDeletedRow())
                font.setItalic(true);

            return font;
        }
        case Qt::ToolTipRole:
        {
            if (!CFG_UI.General.ShowDataViewTooltips.get() || getModel()->getView()->getSimpleBrowserMode())
                return QVariant();

            return getToolTip();
        }
    }

    return QStandardItem::data(role);
}

void SqlQueryItem::resetInitialFocusSelection()
{
    QStandardItem::setData(QVariant(), DataRole::EDIT_SKIP_INITIAL_SELECT);

}

void SqlQueryItem::skipInitialFocusSelection()
{
    QStandardItem::setData(true, DataRole::EDIT_SKIP_INITIAL_SELECT);
}

bool SqlQueryItem::shoulSkipInitialFocusSelection() const
{
    return QStandardItem::data(DataRole::EDIT_SKIP_INITIAL_SELECT).toBool();
}
