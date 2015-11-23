#include "sqlqueryitemdelegate.h"
#include "sqlquerymodel.h"
#include "sqlqueryitem.h"
#include "common/unused.h"
#include "services/notifymanager.h"
#include "sqlqueryview.h"
#include "uiconfig.h"
#include "common/utils_sql.h"
#include <QHeaderView>
#include <QPainter>
#include <QEvent>
#include <QLineEdit>
#include <QDebug>
#include <QComboBox>

SqlQueryItemDelegate::SqlQueryItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void SqlQueryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    SqlQueryItem* item = getItem(index);

    if (item->isUncommited())
    {
        painter->setPen(item->isCommitingError() ? CFG_UI.Colors.DataUncommitedError.get() : CFG_UI.Colors.DataUncommited.get());
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
    }
}

QWidget* SqlQueryItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    UNUSED(option);
    if (!index.isValid())
        return nullptr;

    const SqlQueryModel* model = dynamic_cast<const SqlQueryModel*>(index.model());
    SqlQueryItem* item = model->itemFromIndex(index);

    if (item->isDeletedRow())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %2").arg(tr("The row is marked for deletion.")));
        return nullptr;
    }

    if (!item->getColumn()->canEdit())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %2").arg(item->getColumn()->getEditionForbiddenReason()));
        return nullptr;
    }

    if (item->isLimitedValue())
        item->loadFullData();

    if (!item->getColumn()->getFkConstraints().isEmpty())
        return getFkEditor(item, parent);

    return getEditor(item->getValue().userType(), parent);
}

QString SqlQueryItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    UNUSED(locale);

    if (value.type() == QVariant::Double)
        return value.toString();

    return QStyledItemDelegate::displayText(value, locale);
}

void SqlQueryItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* cb = dynamic_cast<QComboBox*>(editor);
    if (!cb) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    setEditorDataForFk(cb, index);
}

void SqlQueryItemDelegate::setEditorDataForFk(QComboBox* cb, const QModelIndex& index) const
{
    const SqlQueryModel* queryModel = dynamic_cast<const SqlQueryModel*>(index.model());
    SqlQueryItem* item = queryModel->itemFromIndex(index);
    QVariant modelData = item->getValue();
    int idx = cb->findData(modelData, Qt::UserRole);
    if (idx == -1 )
    {
        cb->addItem(modelData.toString(), modelData);
        idx = cb->count() - 1;

        QTableView* view = dynamic_cast<QTableView*>(cb->view());
        view->resizeColumnsToContents();
        view->setMinimumWidth(view->horizontalHeader()->length());
    }
    cb->setCurrentIndex(idx);
}

void SqlQueryItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* cb = dynamic_cast<QComboBox*>(editor);
    if (!cb) {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    setModelDataForFk(cb, model, index);
}

void SqlQueryItemDelegate::setModelDataForFk(QComboBox* cb, QAbstractItemModel* model, const QModelIndex& index) const
{
    QVariant comboData = cb->currentData();
    if (cb->currentText() != cb->itemText(cb->currentIndex()))
        comboData = cb->currentText();

    model->setData(index, comboData);
}

SqlQueryItem* SqlQueryItemDelegate::getItem(const QModelIndex &index) const
{
    const SqlQueryModel* queryModel = dynamic_cast<const SqlQueryModel*>(index.model());
    return queryModel->itemFromIndex(index);
}

QWidget* SqlQueryItemDelegate::getEditor(int type, QWidget* parent) const
{
    UNUSED(type);
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(editor->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, editor));
    return editor;

}

QString SqlQueryItemDelegate::getSqlForFkEditor(SqlQueryItem* item) const
{
    QString sql = QStringLiteral("SELECT %1 FROM %2%3");
    QStringList selCols;
    QStringList fkTables;
    QStringList fkCols;
    Db* db = item->getModel()->getDb();
    Dialect dialect = db->getDialect();

    QList<SqlQueryModelColumn::ConstraintFk*> fkList = item->getColumn()->getFkConstraints();
    int i = 0;
    QString src;
    QString col;
    for (SqlQueryModelColumn::ConstraintFk* fk : fkList)
    {
        col = wrapObjIfNeeded(fk->foreignColumn, dialect);
        src = wrapObjIfNeeded(fk->foreignTable, dialect);
        if (i == 0)
        {
            selCols << QString("%1.%2 AS %3").arg(src, col,
                wrapObjIfNeeded(item->getColumn()->column, dialect));
        }

        selCols << src + ".*";
        fkCols << col;
        fkTables << src;

        i++;
    }

    QStringList conditions;
    QString firstSrc = wrapObjIfNeeded(fkTables.first(), dialect);
    QString firstCol = wrapObjIfNeeded(fkCols.first(), dialect);
    for (i = 1; i < fkTables.size(); i++)
    {
        src = wrapObjIfNeeded(fkTables[i], dialect);
        col = wrapObjIfNeeded(fkCols[i], dialect);
        conditions << QString("%1.%2 = %3.%4").arg(firstSrc, firstCol, src, col);
    }

    QString conditionsStr;
    if (!conditions.isEmpty()) {
        conditionsStr = " WHERE " + conditions.join(", ");
    }

    return sql.arg(selCols.join(", "), fkTables.join(", "), conditionsStr);
}

void SqlQueryItemDelegate::copyToModel(const SqlQueryPtr& results, QStandardItemModel* model) const
{
    QList<SqlResultsRowPtr> rows = results->getAll();
    int colCount = results->columnCount();
    int rowCount = rows.size() + 1;

    model->setColumnCount(colCount);
    model->setRowCount(rowCount);
    int colIdx = 0;
    int rowIdx = 0;
    for (const QString& colName : results->getColumnNames())
    {
        model->setHeaderData(colIdx, Qt::Horizontal, colName);

        QStandardItem *item = new QStandardItem();
        QFont font = item->font();
        font.setItalic(true);
        item->setFont(font);
        item->setForeground(QBrush(CFG_UI.Colors.DataNullFg.get()));
        item->setData(QVariant(QVariant::String), Qt::EditRole);
        item->setData(QVariant(QVariant::String), Qt::UserRole);
        model->setItem(0, colIdx, item);
        colIdx++;
    }
    rowIdx++;

    for (const SqlResultsRowPtr& row : rows)
    {
        colIdx = 0;
        for (const QVariant& val : row->valueList())
        {
            QStandardItem *item = new QStandardItem();
            item->setText(val.toString());
            item->setData(val, Qt::UserRole);
            model->setItem(rowIdx, colIdx, item);
            colIdx++;
        }
        rowIdx++;
    }
}

QWidget* SqlQueryItemDelegate::getFkEditor(SqlQueryItem* item, QWidget* parent) const
{
    QString sql = getSqlForFkEditor(item);

    QComboBox *cb = new QComboBox(parent);
    cb->setEditable(true);
    QTableView* queryView = new QTableView();
    QStandardItemModel* model = new QStandardItemModel(queryView);

    Db* db = item->getModel()->getDb();
    SqlQueryPtr results = db->exec(sql);
    copyToModel(results, model);

    cb->setModel(model);
    cb->setView(queryView);
    cb->setModelColumn(0);

    queryView->verticalHeader()->setVisible(false);
    queryView->horizontalHeader()->setVisible(true);
    queryView->setSelectionMode(QAbstractItemView::SingleSelection);
    queryView->setSelectionBehavior(QAbstractItemView::SelectRows);
    queryView->resizeColumnsToContents();
    queryView->resizeRowsToContents();
    queryView->setMinimumWidth(queryView->horizontalHeader()->length());

    return cb;
}
