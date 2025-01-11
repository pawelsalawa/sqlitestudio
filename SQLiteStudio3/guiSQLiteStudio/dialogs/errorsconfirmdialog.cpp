#include "errorsconfirmdialog.h"
#include "ui_errorsconfirmdialog.h"
#include "iconmanager.h"

ErrorsConfirmDialog::ErrorsConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorsConfirmDialog)
{
    ui->setupUi(this);
}

ErrorsConfirmDialog::~ErrorsConfirmDialog()
{
    delete ui;
}

void ErrorsConfirmDialog::setErrors(const QHash<QString,QSet<QString>>& errors)
{
    ui->list->clear();

    for (const QString& key : errors.keys())
    {
        for (const QString& err : errors[key])
            ui->list->addItem(QString("[%1] %2").arg(key, err));
    }

    for (int i = 0, total = ui->list->count(); i < total; ++i)
        ui->list->item(i)->setIcon(ICONS.STATUS_ERROR);
}

void ErrorsConfirmDialog::setErrors(const QSet<QString>& errors)
{
    ui->list->clear();
    ui->list->addItems(errors.values());
    for (int i = 0, total = ui->list->count(); i < total; ++i)
        ui->list->item(i)->setIcon(ICONS.STATUS_ERROR);
}

void ErrorsConfirmDialog::setTopLabel(const QString& text)
{
    ui->topLabel->setText(text);
}

void ErrorsConfirmDialog::setBottomLabel(const QString& text)
{
    ui->bottomLabel->setText(text);
}
