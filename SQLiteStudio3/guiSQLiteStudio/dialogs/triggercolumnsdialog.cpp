#include "triggercolumnsdialog.h"
#include "ui_triggercolumnsdialog.h"
#include "uiutils.h"
#include <QCheckBox>

TriggerColumnsDialog::TriggerColumnsDialog(QWidget *parent, int globalX, int globalY) :
    QDialog(parent, Qt::Popup),
    globalX(globalX),
    globalY(globalY),
    ui(new Ui::TriggerColumnsDialog)
{
    ui->setupUi(this);
}

TriggerColumnsDialog::~TriggerColumnsDialog()
{
    delete ui;
}

void TriggerColumnsDialog::addColumn(const QString& name, bool checked)
{
    QCheckBox* cb = new QCheckBox(name);
    cb->setChecked(checked);
    ui->mainWidget->layout()->addWidget(cb);
    checkBoxList << cb;
}

QStringList TriggerColumnsDialog::getCheckedColumns() const
{
    QStringList columns;
    foreach (QCheckBox* cb, checkBoxList)
    {
        if (cb->isChecked())
            columns << cb->text();
    }
    return columns;
}

void TriggerColumnsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void TriggerColumnsDialog::showEvent(QShowEvent*)
{
    adjustSize();
    move(globalX, globalY);
}
