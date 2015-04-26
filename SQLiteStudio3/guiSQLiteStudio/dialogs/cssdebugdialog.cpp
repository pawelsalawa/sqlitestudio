#include "cssdebugdialog.h"
#include "ui_cssdebugdialog.h"
#include "mainwindow.h"
#include <QApplication>

CssDebugDialog::CssDebugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CssDebugDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
    ui->cssEdit->setPlainText(MAINWINDOW->styleSheet());
}

CssDebugDialog::~CssDebugDialog()
{
    delete ui;
}

void CssDebugDialog::buttonClicked(QAbstractButton* button)
{
    if (ui->buttonBox->buttonRole(button) != QDialogButtonBox::ApplyRole)
        return;

    MAINWINDOW->setStyleSheet(ui->cssEdit->toPlainText());
}

void CssDebugDialog::closeEvent(QCloseEvent*)
{
    deleteLater();
}
