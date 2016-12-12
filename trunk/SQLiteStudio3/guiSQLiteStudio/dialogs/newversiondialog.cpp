#ifdef PORTABLE_CONFIG

#include "newversiondialog.h"
#include "services/pluginmanager.h"
#include "sqlitestudio.h"
#include "ui_newversiondialog.h"
#include "services/config.h"
#include <QInputDialog>

NewVersionDialog::NewVersionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewVersionDialog)
{
    init();
}

NewVersionDialog::~NewVersionDialog()
{
    delete ui;
}

void NewVersionDialog::setUpdates(const QList<UpdateManager::UpdateEntry>& updates)
{
    QTableWidgetItem* item = nullptr;
    int row = 0;
    ui->updateList->setRowCount(updates.size());
    for (const UpdateManager::UpdateEntry& entry : updates)
    {
        item = new QTableWidgetItem(entry.compontent);
        ui->updateList->setItem(row, 0, item);

        item = new QTableWidgetItem(entry.version);
        ui->updateList->setItem(row, 1, item);

        row++;
    }
    ui->updateList->resizeColumnsToContents();
}

void NewVersionDialog::init()
{
    ui->setupUi(this);

    connect(ui->abortButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(installUpdates()));
    connect(ui->checkOnStartupCheck, &QCheckBox::clicked, [=](bool checked)
    {
        CFG_CORE.General.CheckUpdatesOnStartup.set(checked);
    });
}

void NewVersionDialog::installUpdates()
{
    UPDATES->update();
    close();
}

void NewVersionDialog::showEvent(QShowEvent*)
{
    ui->checkOnStartupCheck->setChecked(CFG_CORE.General.CheckUpdatesOnStartup.get());
}

#endif // PORTABLE_CONFIG
