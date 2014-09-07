#include "versionconvertsummarydialog.h"
#include "ui_versionconvertsummarydialog.h"

VersionConvertSummaryDialog::VersionConvertSummaryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VersionConvertSummaryDialog)
{
    ui->setupUi(this);

    ui->diffTable->setLeftLabel(tr("Before"));
    ui->diffTable->setRightLabel(tr("After"));
    ui->diffTable->horizontalHeader()->setVisible(true);
}

VersionConvertSummaryDialog::~VersionConvertSummaryDialog()
{
    delete ui;
}

void VersionConvertSummaryDialog::setSides(const QList<QPair<QString, QString> >& data)
{
    ui->diffTable->setSides(data);
}


void VersionConvertSummaryDialog::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
    ui->diffTable->updateSizes();

}
