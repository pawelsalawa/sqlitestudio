#include "newversiondialog.h"
#include "services/pluginmanager.h"
#include "sqlitestudio.h"
#include "ui_newversiondialog.h"

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
    QTableWidgetItem* item;
    QString currVersion;
    int row = 0;
    ui->updateList->setRowCount(updates.size());
    for (const UpdateManager::UpdateEntry& entry : updates)
    {
        if (entry.compontent == "SQLiteStudio")
            currVersion = SQLITESTUDIO->getVersionString();
        else
            currVersion = PLUGINS->getPrintableVersion(entry.compontent);

        item = new QTableWidgetItem(entry.compontent);
        ui->updateList->setItem(row, 0, item);

        item = new QTableWidgetItem(currVersion);
        ui->updateList->setItem(row, 1, item);

        item = new QTableWidgetItem(entry.version);
        ui->updateList->setItem(row, 2, item);

        row++;
    }
}

void NewVersionDialog::init()
{
    ui->setupUi(this);


}
