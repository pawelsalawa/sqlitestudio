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
        ui->tree->setCurrentItem(firstItem, 0, QItemSelectionModel::Select);
}

QTreeWidgetItem* ErdChangeRegistryDialog::createItemFromChange(int rowIdx, ErdChange* change)
{
    static_qstring(topLabelTpl, "%1. %2");

    QString ddl = FORMATTER->format("sql", change->toDdl(true).join("\n\n"), db);
    QString label = rowIdx >= 0 ?
                topLabelTpl.arg(rowIdx + 1).arg(change->getDescription()) :
                change->getDescription();

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, label);
    item->setText(1, ddl);
    return item;
}

void ErdChangeRegistryDialog::handleItemSelected(QTreeWidgetItem* item)
{
    ui->previewEdit->setContents(item->text(1));
}
