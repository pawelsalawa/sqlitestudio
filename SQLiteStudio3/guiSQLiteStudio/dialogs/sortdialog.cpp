#include "sortdialog.h"
#include "ui_sortdialog.h"
#include "iconmanager.h"
#include "common/unused.h"
#include <QComboBox>
#include <QDebug>
#include <QPushButton>

SortDialog::SortDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SortDialog)
{
    ui->setupUi(this);

    initActions();
    ui->list->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));
    connect(ui->list, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemChanged(QTreeWidgetItem*,int)));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(reset()));
    connect(ui->list->model(), &QAbstractItemModel::rowsInserted, [=, this](const QModelIndex & parent, int start, int end)
    {
        UNUSED(parent);
        UNUSED(end);
        rebuildComboForItem(ui->list->topLevelItem(start));
    });
}

SortDialog::~SortDialog()
{
    delete ui;
}

void SortDialog::setColumns(const QStringList& columns)
{
    originalColumns = columns;
    ui->list->clear();

    QTreeWidgetItem* item = nullptr;
    for (int row = 0, total = columns.size(); row < total; ++row)
    {
        item = new QTreeWidgetItem({columns[row], "ASC"});
        item->setData(2, Qt::UserRole, row);
        fixItemFlags(item);
        ui->list->insertTopLevelItem(row, item);
        item->setCheckState(0, Qt::Unchecked);
    }
    ui->list->setHeaderLabels({tr("Column"), tr("Order")});
    updateButtons();
}

QueryExecutor::SortList SortDialog::getSortOrder() const
{
    QueryExecutor::SortList sortOrder;

    QTreeWidgetItem* item = nullptr;
    QComboBox* combo = nullptr;
    for (int row = 0, total = ui->list->topLevelItemCount(); row < total; ++row)
    {
        item = ui->list->topLevelItem(row);
        if (item->checkState(0) != Qt::Checked)
            continue;

        combo = dynamic_cast<QComboBox*>(ui->list->itemWidget(item, 1));
        sortOrder << QueryExecutor::Sort((combo->currentText() == "ASC" ? Qt::AscendingOrder : Qt::DescendingOrder), item->data(2, Qt::UserRole).toInt());
    }
    return sortOrder;
}

void SortDialog::setSortOrder(const QueryExecutor::SortList& sortOrder)
{
    // Translate sort order into more usable (in here) form
    QHash<int,QueryExecutor::Sort::Order> checkedColumns;
    QList<int> checkedColumnsOrder;
    for (const QueryExecutor::Sort& sort : sortOrder)
    {
        checkedColumns[sort.column] = sort.order;
        checkedColumnsOrder << sort.column;
    }

    // Select proper columns and set order
    bool checked;
    QTreeWidgetItem* item = nullptr;
    QComboBox* combo = nullptr;
    for (int row = 0, total = ui->list->topLevelItemCount(); row < total; ++row)
    {
        item = ui->list->topLevelItem(row);
        checked = checkedColumns.contains(item->data(2, Qt::UserRole).toInt());
        item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);

        combo = dynamic_cast<QComboBox*>(ui->list->itemWidget(item, 1));
        combo->setCurrentText(checkedColumns[row] == QueryExecutor::Sort::DESC ? "DESC" : "ASC");
    }

    // Get selected items as an ordered list of items (in order as defined in the sort order), so we can easly relocate them
    QList<QTreeWidgetItem*> orderedItems;
    for (int row : checkedColumnsOrder)
        orderedItems << ui->list->topLevelItem(row);

    // Move selected items in front, in the same order as they were mentioned in the sort order
    int newRow = 0;
    for (QTreeWidgetItem* itemToMove : orderedItems)
    {
        ui->list->takeTopLevelItem(ui->list->indexOfTopLevelItem(itemToMove));
        ui->list->insertTopLevelItem(newRow++, itemToMove);
    }

    updateState();
}

QToolBar* SortDialog::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}

void SortDialog::updateState(QTreeWidgetItem* item)
{
    QComboBox* combo = dynamic_cast<QComboBox*>(ui->list->itemWidget(item, 1));
    if (!combo)
        return;

    combo->setEnabled(item->checkState(0) == Qt::Checked);
}

void SortDialog::updateState()
{
    for (int row = 0, total = ui->list->topLevelItemCount(); row < total; ++row)
        updateState(ui->list->topLevelItem(row));
}

void SortDialog::fixItemFlags(QTreeWidgetItem* item)
{
    Qt::ItemFlags flags = item->flags();
    flags |= Qt::ItemNeverHasChildren;
    flags |= Qt::ItemIsUserCheckable;
    flags ^= Qt::ItemIsDropEnabled;
    flags ^= Qt::ItemIsEditable;
    item->setFlags(flags);
}

void SortDialog::rebuildComboForItem(QTreeWidgetItem* item)
{
    QComboBox* combo = new QComboBox();
    combo->addItems({"ASC", "DESC"});
    combo->setCurrentText(item->text(1));
    combo->setEnabled(item->checkState(0) == Qt::Checked);
    ui->list->setItemWidget(item, 1, combo);
    item->setSizeHint(1, combo->sizeHint()); // bug in Qt? without this comboboxes were misaligned vertically

    connect(combo, &QComboBox::currentTextChanged, [item](const QString& newText)
    {
        item->setText(1, newText);
    });

    updateSortLabel();
}

void SortDialog::updateSortLabel()
{
    QStringList entries;
    QTreeWidgetItem* item = nullptr;
    for (int row = 0, total = ui->list->topLevelItemCount(); row < total; ++row)
    {
        item = ui->list->topLevelItem(row);
        if (item->checkState(0) != Qt::Checked)
            continue;

        entries << item->text(0) + " " + item->text(1);
    }

    if (entries.size() == 0)
    {
        ui->sortByLabel->setVisible(false);
    }
    else
    {
        static QString label = tr("Sort by: %1");
        ui->sortByLabel->setText(label.arg(entries.join(", ")));
        ui->sortByLabel->setVisible(true);
    }
}

void SortDialog::itemChanged(QTreeWidgetItem* item, int column)
{
    if (column == 0)
        updateState(item);

    updateSortLabel();
}

void SortDialog::reset()
{
    setColumns(originalColumns);
}

void SortDialog::updateButtons()
{
    QTreeWidgetItem* item = ui->list->currentItem();
    actionMap[MOVE_UP]->setEnabled(item && ui->list->itemAbove(item) != nullptr);
    actionMap[MOVE_DOWN]->setEnabled(item && ui->list->itemBelow(item) != nullptr);
}

void SortDialog::moveCurrentUp()
{
    QTreeWidgetItem* item = ui->list->currentItem();
    if (!item)
        return;

    int row = ui->list->indexOfTopLevelItem(item);
    if (row < 1)
        return;

    ui->list->takeTopLevelItem(row);
    ui->list->insertTopLevelItem(row - 1, item);

    QModelIndex idx = ui->list->model()->index(row - 1, 0);
    ui->list->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
    updateButtons();
}

void SortDialog::moveCurrentDown()
{
    QTreeWidgetItem* item = ui->list->currentItem();
    if (!item)
        return;

    int row = ui->list->indexOfTopLevelItem(item);
    if (row + 1 >= ui->list->topLevelItemCount())
        return;

    ui->list->takeTopLevelItem(row);
    ui->list->insertTopLevelItem(row + 1, item);

    QModelIndex idx = ui->list->model()->index(row + 1, 0);
    ui->list->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
    updateButtons();
}

void SortDialog::createActions()
{
    createAction(MOVE_UP, ICONS.MOVE_UP, tr("Move column up"), this, SLOT(moveCurrentUp()), ui->toolbar, this);
    createAction(MOVE_DOWN, ICONS.MOVE_DOWN, tr("Move column down"), this, SLOT(moveCurrentDown()), ui->toolbar, this);
}

void SortDialog::setupDefShortcuts()
{
}
