#include "erdchange.h"
#include "erdchangecomposite.h"
#include "erdchangeregistry.h"
#include "erdchangeregistrydialog.h"
#include "erdeffectivechangemerger.h"
#include "ui_erdchangeregistrydialog.h"
#include "services/codeformatter.h"

ErdChangeRegistryDialog::ErdChangeRegistryDialog(Db* db, ErdChangeRegistry* changeRegistry,
                                                 const QStringList& schemaBase, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErdChangeRegistryDialog),
    db(db),
    changeRegistry(changeRegistry),
    schemaBase(schemaBase)
{
    ui->setupUi(this);
    ui->tree->setDragEnabled(false);
    ui->tree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);

    connect(ui->tree, &QTreeWidget::currentItemChanged, this, &ErdChangeRegistryDialog::handleItemSelected);
    connect(ui->compactCheckBox, &QCheckBox::toggled, this, &ErdChangeRegistryDialog::compactViewToggled);
    connect(ui->noDdlCheckBox, &QCheckBox::toggled, this, &ErdChangeRegistryDialog::applyFiltering);

    populateTree();
}

ErdChangeRegistryDialog::~ErdChangeRegistryDialog()
{
    delete ui;
}

void ErdChangeRegistryDialog::populateTree()
{
    ui->tree->clear();

    int rowIdx = 0;
    int labelIdx = 1;
    for (ErdChange*& chg : changeRegistry->getPendingChanges(true))
    {
        QTreeWidgetItem* item = createItemFromChange(labelIdx, chg);
        ui->tree->insertTopLevelItem(rowIdx, item);
        rowIdx++;
    }
    ui->tree->header()->resizeSections(QHeaderView::ResizeToContents);
    applyFiltering();
}

QTreeWidgetItem* ErdChangeRegistryDialog::createItemFromChange(int& labelIdx, ErdChange* change)
{
    static_qstring(topLabelTpl, "%1. %2");

    bool ddlChange = change->isDdlChange();
    QString ddl = ddlChange ?
                FORMATTER->format("sql", change->toDdl(true).join("\n\n"), db) :
                tr("-- This is a change applied only to the diagram. It has no database schema efects.", "ERD editor");
    QString label = labelIdx >= 0 && ddlChange ?
                topLabelTpl.arg(labelIdx++).arg(change->getDescription()) :
                change->getDescription();

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, label);
    item->setData(0, DDL, ddl);
    item->setData(0, IS_DDL_CHANGE, ddlChange);
    if (!ddlChange)
    {
        auto f = item->font(0);
        f.setItalic(true);
        item->setFont(0, f);
    }

    ErdChangeComposite* composite = dynamic_cast<ErdChangeComposite*>(change);
    if (composite)
    {
        int childIdx = 0;
        int childLabelIdx = -1;
        for (auto&& childChg : composite->getChanges())
        {
            QTreeWidgetItem* childItem = createItemFromChange(childLabelIdx, childChg);
            item->insertChild(childIdx++, childItem);
        }
    }

    return item;
}

void ErdChangeRegistryDialog::populateEffectiveChanges()
{
    ui->tree->clear();

    ErdEffectiveChangeMerger merger(schemaBase, db->getName());
    QList<ErdEffectiveChange> effectiveChanges = merger.merge(changeRegistry->getPendingChanges(false));

    int rowIdx = 0;
    int labelIdx = 1;
    for (const ErdEffectiveChange& chg : effectiveChanges)
    {
        QTreeWidgetItem* item = createItemFromChange(labelIdx, chg, merger);
        ui->tree->insertTopLevelItem(rowIdx, item);
        rowIdx++;
    }
    ui->tree->header()->resizeSections(QHeaderView::ResizeToContents);
    selectFirstVisibleItem();
}

QTreeWidgetItem* ErdChangeRegistryDialog::createItemFromChange(int& labelIdx, const ErdEffectiveChange& change,
                                                               const ErdEffectiveChangeMerger& merger)
{
    static_qstring(topLabelTpl, "%1. %2");

    QString ddl = FORMATTER->format("sql", merger.getDdlForChange(change).join("\n\n"), db);
    QString label = topLabelTpl.arg(labelIdx++).arg(change.getDescription());

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, label);
    item->setData(0, DDL, ddl);
    return item;
}

void ErdChangeRegistryDialog::applyFiltering()
{
    if (ui->noDdlCheckBox->isChecked())
    {
        for (QTreeWidgetItemIterator it(ui->tree); *it; ++it)
            (*it)->setHidden(false);
    }
    else
    {
        for (QTreeWidgetItemIterator it(ui->tree); *it; ++it)
        {
            (*it)->setHidden(!(*it)->data(0, IS_DDL_CHANGE).toBool());
        }
    }
    selectFirstVisibleItem();
}

void ErdChangeRegistryDialog::selectFirstVisibleItem()
{
    if (!ui->tree->currentItem() || ui->tree->currentItem()->isHidden())
    {
        const int count = ui->tree->topLevelItemCount();
        for (int i = 0; i < count; ++i) {
            QTreeWidgetItem* item = ui->tree->topLevelItem(i);
            if (!item->isHidden())
            {
                ui->tree->setCurrentItem(item, 0, QItemSelectionModel::ClearAndSelect);
                break;
            }
        }
        if (count == 0)
            ui->previewEdit->clear();
    }
}

void ErdChangeRegistryDialog::handleItemSelected(QTreeWidgetItem* item)
{
    if (item)
        ui->previewEdit->setContents(item->data(0, DDL).toString());
}

void ErdChangeRegistryDialog::compactViewToggled(bool enabled)
{
    if (enabled)
        ui->noDdlCheckBox->setChecked(false);

    ui->noDdlCheckBox->setEnabled(!enabled);
    if (enabled)
        populateEffectiveChanges();
    else
        populateTree();
}
