#include "sqlqueryitem.h"
#include "common/utils_sql.h"
#include "sqlquerymodel.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include <QDate>
#include <QDebug>

SqlQueryItem::SqlQueryItem(QObject *parent) :
    QObject(parent)
{
    setUncommited(false);
    setCommitingError(false);
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

bool SqlQueryItem::isUncommited() const
{
    return QStandardItem::data(DataRole::UNCOMMITED).toBool();
}

void SqlQueryItem::setUncommited(bool uncommited)
{
    QStandardItem::setData(QVariant(uncommited), DataRole::UNCOMMITED);
    if (!uncommited)
    {
        setOldValue(QVariant());
        setCommitingError(false);
    }
}

void SqlQueryItem::rollback()
{
    setValue(getOldValue(), true, true);
    setUncommited(false);
    setDeletedRow(false);
}

bool SqlQueryItem::isCommitingError() const
{
    return QStandardItem::data(DataRole::COMMITING_ERROR).toBool();
}

void SqlQueryItem::setCommitingError(bool isError)
{
    QStandardItem::setData(QVariant(isError), DataRole::COMMITING_ERROR);
}

bool SqlQueryItem::isNewRow() const
{
    return QStandardItem::data(DataRole::NEW_ROW).toBool();
}

void SqlQueryItem::setNewRow(bool isNew)
{
    QStandardItem::setData(QVariant(isNew), DataRole::NEW_ROW);
}

bool SqlQueryItem::isJustInsertedWithOutRowId() const
{
    return QStandardItem::data(DataRole::JUST_INSERTED_WITHOUT_ROWID).toBool();
}

void SqlQueryItem::setJustInsertedWithOutRowId(bool justInsertedWithOutRowId)
{
    QStandardItem::setData(QVariant(justInsertedWithOutRowId), DataRole::JUST_INSERTED_WITHOUT_ROWID);
}

bool SqlQueryItem::isDeletedRow() const
{
    return QStandardItem::data(DataRole::DELETED).toBool();
}

void SqlQueryItem::setDeletedRow(bool isDeleted)
{
    if (isDeleted)
        setOldValue(getValue());

    QStandardItem::setData(QVariant(isDeleted), DataRole::DELETED);
}

QVariant SqlQueryItem::getValue() const
{
    return QStandardItem::data(DataRole::VALUE);
}

void SqlQueryItem::setValue(const QVariant &value, bool limited, bool loadedFromDb)
{
    QVariant newValue = adjustVariantType(value);
    QVariant origValue = getValue();

    // It's modified when:
    // - original and new value is different (value or NULL status), while it's not loading from DB
    // - this item was already marked as uncommited
    bool modified = (
                        (
                                newValue != origValue ||
                                origValue.isNull() != newValue.isNull()
                        ) &&
                        !loadedFromDb
                    ) ||
                    isUncommited();

    if (modified && !getOldValue().isValid())
        setOldValue(origValue);

    // This is a workaround for an issue in Qt, that uses operator== to compare values in QStandardItem::setData().
    // If the old value is null and the new value is empty, then operator == returns true, which is a lie.
    // We need to trick QStandardItem to force updating the value. We feed it with any non-empty value,
    // then we can set whatever value we want and it should be updated (or not, if it's truly the same value).
    QStandardItem::setData("x", DataRole::VALUE);

    QStandardItem::setData(newValue, DataRole::VALUE);
    setLimitedValue(limited);
    setUncommited(modified);

    if (modified)
        getModel()->itemValueEdited(this);
}

bool SqlQueryItem::isLimitedValue() const
{
    return QStandardItem::data(DataRole::LIMITED_VALUE).toBool();
}

QVariant SqlQueryItem::getOldValue() const
{
    return QStandardItem::data(DataRole::OLD_VALUE);
}

void SqlQueryItem::setOldValue(const QVariant& value)
{
    QStandardItem::setData(value, DataRole::OLD_VALUE);
}

void SqlQueryItem::setLimitedValue(bool limited)
{
    QStandardItem::setData(QVariant(limited), DataRole::LIMITED_VALUE);
}

QVariant SqlQueryItem::adjustVariantType(const QVariant& value)
{
    QVariant newValue;
    bool ok;
    newValue = value.toInt(&ok);
    if (ok)
        return newValue;

    newValue = value.toLongLong(&ok);
    if (ok)
        return newValue;

    newValue = value.toDouble(&ok);
    if (ok)
        return newValue;

    return value;
}

QString SqlQueryItem::getToolTip() const
{
    static const QString tableTmp = "<table>%1</table>";
    static const QString rowTmp = "<tr><td colspan=2 style=\"white-space: pre\">%1</td><td style=\"align: right\"><b>%2</b></td></tr>";
    static const QString hdrRowTmp = "<tr><td width=16><img src=\"%1\"/></td><th colspan=2 style=\"align: center\">%2 %3</th></tr>";
    static const QString constrRowTmp = "<tr><td width=16><img src=\"%1\"/></td><td style=\"white-space: pre\"><b>%2</b></td><td>%3</td></tr>";
    static const QString emptyRow = "<tr><td colspan=3></td></tr>";

    if (!index().isValid())
        return QString::null;

    SqlQueryModelColumn* col = getColumn();
    if (!col)
        return QString::null; // happens when simple execution method was performed

    QStringList rows;
    rows << hdrRowTmp.arg(ICONS.COLUMN.getPath()).arg(tr("Column:", "data view tooltip")).arg(col->column);
    rows << rowTmp.arg(tr("Data type:", "data view")).arg(col->dataType.typeStr);
    if (!col->table.isNull())
    {
        rows << rowTmp.arg(tr("Table:", "data view tooltip")).arg(col->table);

        RowId rowId = getRowId();
        QString rowIdStr;
        if (rowId.size() == 1)
        {
            rowIdStr = rowId.values().first().toString();
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
        rows << rowTmp.arg("ROWID:").arg(rowIdStr);
    }

    if (col->constraints.size() > 0)
    {
        rows << emptyRow;
        rows << hdrRowTmp.arg(ICONS.COLUMN_CONSTRAINT.getPath()).arg(tr("Constraints:", "data view tooltip")).arg("");
        foreach (SqlQueryModelColumn::Constraint* constr, col->constraints)
            rows << constrRowTmp.arg(constr->getIcon()->toUrl()).arg(constr->getTypeString()).arg(constr->getDetails());
    }

    return tableTmp.arg(rows.join(""));
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
            setValue(value, false, (column() == -1));
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
            if (value.isNull())
                return "NULL";

            return value;
        }
        case Qt::ForegroundRole:
        {
            QVariant value = getValue();
            if (value.isNull())
                return QBrush(CFG_UI.Colors.DataNullFg.get());

            break;
        }
        case Qt::BackgroundRole:
        {
            if (isDeletedRow())
                return QBrush(CFG_UI.Colors.DataDeletedBg.get());

            break;
        }
        case Qt::TextAlignmentRole:
        {
            QVariant value = getValue();
            if (value.isNull() || isDeletedRow())
                return Qt::AlignCenter;

            break;
        }
        case Qt::FontRole:
        {
            QFont font = CFG_UI.Fonts.DataView.get();

            QVariant value = getValue();
            if (value.isNull() || isDeletedRow())
                font.setItalic(true);

            return font;
        }
        case Qt::ToolTipRole:
        {
            return getToolTip();
        }
    }

    return QStandardItem::data(role);
}

QString SqlQueryItem::loadFullData()
{
    SqlQueryModelColumn* col = getColumn();
    if (col->editionForbiddenReason.size() > 0)
    {
        qWarning() << "Tried to load full cell which is not editable. This should be already handled in Editor class when invoking edition action.";
        return tr("This cell is not editable, because: %1").arg(SqlQueryModelColumn::resolveMessage(col->editionForbiddenReason.values().first()));
    }

    if (isJustInsertedWithOutRowId())
    {
        QString msg = tr("When inserted new row to the WITHOUT ROWID table, using DEFAULT value for PRIMARY KEY, "
                         "the table has to be reloaded in order to edit the new row.");
        return tr("This cell is not editable, because: %1").arg(msg);
    }

    SqlQueryModel *model = getModel();
    Db* db = model->getDb();
    if (!db->isOpen())
    {
        qWarning() << "Tried to load the data for a cell that refers to the already closed database.";
        return tr("Cannot load the data for a cell that refers to the already closed database.");
    }

    Dialect dialect = db->getDialect();

    // Query
    QString query = "SELECT %1 FROM %2 WHERE ROWID = :rowId";

    // Column
    query = query.arg(wrapObjIfNeeded(col->column, dialect));

    // Database and table
    QString source = wrapObjIfNeeded(col->table, dialect);
    if (!col->database.isNull())
        source.prepend(wrapObjIfNeeded(col->database, dialect)+".");

    query = query.arg(source);

    // Get the data
    SqlResultsPtr results = db->exec(query, {getRowId()});
    if (results->isError())
        return results->getErrorText();

    setValue(results->getSingleCell(), false, true);
    return QString::null;
}

QVariant SqlQueryItem::getFullValue()
{
    if (!isLimitedValue())
        return getValue();

    QVariant originalValue = getValue();
    loadFullData();
    QVariant result = getValue();
    setValue(originalValue, true, !isUncommited());
    return result;
}
