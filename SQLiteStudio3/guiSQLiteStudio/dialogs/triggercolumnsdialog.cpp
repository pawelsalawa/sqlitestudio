#include "triggercolumnsdialog.h"
#include "ui_triggercolumnsdialog.h"
#include <QCheckBox>

TriggerColumnsDialog::TriggerColumnsDialog(QWidget *parent, int globalX, int globalY) :
#ifdef Q_OS_OSX
    QDialog(parent),
#else
    QDialog(parent, Qt::Popup),
#endif
    globalX(globalX),
    globalY(globalY),
    ui(new Ui::TriggerColumnsDialog)
{
    ui->setupUi(this);

    connect(ui->selectAllBtn, SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(ui->deselectAllBtn, SIGNAL(clicked(bool)), this, SLOT(deselectAll()));
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
    for (QCheckBox* cb : checkBoxList)
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

void TriggerColumnsDialog::selectAll()
{
    for (QCheckBox* cb : checkBoxList)
        cb->setChecked(true);
}

void TriggerColumnsDialog::deselectAll()
{
    for (QCheckBox* cb : checkBoxList)
        cb->setChecked(false);
}
