#include "bugreportlogindialog.h"
#include "ui_bugreportlogindialog.h"

BugReportLoginDialog::BugReportLoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BugReportLoginDialog)
{
    init();
}

BugReportLoginDialog::~BugReportLoginDialog()
{
    delete ui;
}

void BugReportLoginDialog::init()
{
    ui->setupUi(this);

}
