#include "fileexecerrorsdialog.h"
#include "ui_fileexecerrorsdialog.h"

FileExecErrorsDialog::FileExecErrorsDialog(const QList<QPair<QString, QString>>& errors, bool rolledBack, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileExecErrorsDialog)
{
    ui->setupUi(this);

    ui->committedLabel->setVisible(!rolledBack);
    ui->rolledBackLabel->setVisible(rolledBack);

    ui->tableWidget->setRowCount(errors.size());
    int row = 0;
    for (const QPair<QString, QString>& err : errors)
    {
        ui->tableWidget->setItem(row, 0, item(err.first));
        ui->tableWidget->setItem(row, 1, item(err.second));
        row++;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->resizeRowsToContents();
}

FileExecErrorsDialog::~FileExecErrorsDialog()
{
    delete ui;
}

QTableWidgetItem* FileExecErrorsDialog::item(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(Qt::ItemIsEnabled);
    return item;
}
