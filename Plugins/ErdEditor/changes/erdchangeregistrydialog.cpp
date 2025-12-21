#include "erdchange.h"
#include "erdchangecomposite.h"
#include "erdchangeregistry.h"
#include "erdchangeregistrydialog.h"
#include "ui_erdchangeregistrydialog.h"
#include "services/codeformatter.h"

ErdChangeRegistryDialog::ErdChangeRegistryDialog(Db* db, ErdChangeRegistry* changeRegistry, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErdChangeRegistryDialog),
    db(db),
    changeRegistry(changeRegistry)
{
    ui->setupUi(this);
    ui->tree->setDragEnabled(false);
    ui->tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->tree, &QTreeWidget::currentItemChanged, this, &ErdChangeRegistryDialog::handleItemSelected);

    populateTree();
}

ErdChangeRegistryDialog::~ErdChangeRegistryDialog()
{
    delete ui;
}

void ErdChangeRegistryDialog::populateTree()
{
    QTreeWidgetItem* firstItem = nullptr;
    int rowIdx = 0;
    for (ErdChange*& chg : changeRegistry->getEffectiveChanges())
    {
        QTreeWidgetItem* item = createItemFromChange(rowIdx, chg);
        ui->tree->insertTopLevelItem(rowIdx, item);
        if (!firstItem)
            firstItem = item;

        ErdChangeComposite* composite = dynamic_cast<ErdChangeComposite*>(chg);
        if (composite)
        {
            int childIdx = 0;
            for (auto&& childChg : composite->getChanges())
            {
                QTreeWidgetItem* childItem = createItemFromChange(-1, childChg);
                item->insertChild(childIdx++, childItem);
            }
        }

        rowIdx++;
    }
    ui->tree->header()->resizeSections(QHeaderView::ResizeToContents);
    if (firstItem)
        ui->tree->setCurrentItem(firstItem, 1, QItemSelectionModel::Select);
}

QTreeWidgetItem* ErdChangeRegistryDialog::createItemFromChange(int rowIdx, ErdChange* change)
{
    QString ddl = FORMATTER->format("sql", change->toDdl(true).join("\n\n"), db);

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, rowIdx >= 0 ? QString::number(rowIdx + 1) : QString());
    item->setText(1, change->getDescription());
    item->setText(2, ddl);
    return item;
}

void ErdChangeRegistryDialog::handleItemSelected(QTreeWidgetItem* item)
{
    ui->previewEdit->setContents(item->text(2));
}
