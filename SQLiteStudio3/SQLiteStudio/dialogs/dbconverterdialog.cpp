#include "dbconverterdialog.h"
#include "ui_dbconverterdialog.h"

DbConverterDialog::DbConverterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DbConverterDialog)
{
    ui->setupUi(this);
}

DbConverterDialog::~DbConverterDialog()
{
    delete ui;
}
