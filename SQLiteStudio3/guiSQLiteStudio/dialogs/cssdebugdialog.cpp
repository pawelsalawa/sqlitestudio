#include "cssdebugdialog.h"
#include "ui_cssdebugdialog.h"
#include "mainwindow.h"
#include "themetuner.h"
#include <QApplication>
#include <QPushButton>

CssDebugDialog::CssDebugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CssDebugDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));

    appliedCss = MAINWINDOW->styleSheet();
    ui->cssEdit->setPlainText(appliedCss);
    updateState();

    connect(ui->cssEdit, SIGNAL(textChanged()), this, SLOT(updateState()));
}

CssDebugDialog::~CssDebugDialog()
{
    delete ui;
}

void CssDebugDialog::buttonClicked(QAbstractButton* button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults)
    {
        ui->cssEdit->setPlainText(THEME_TUNER->getDefaultCss());
    }
    else if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
        appliedCss = ui->cssEdit->toPlainText();
        MAINWINDOW->setStyleSheet(appliedCss);
    }

    updateState();
}

void CssDebugDialog::updateState()
{
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(ui->cssEdit->toPlainText() != appliedCss);
}

void CssDebugDialog::closeEvent(QCloseEvent*)
{
    deleteLater();
}
