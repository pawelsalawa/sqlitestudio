#include "erdtablefkpanel.h"
#include "ui_tableforeignkeypanel.h"

ErdTableFkPanel::ErdTableFkPanel(QWidget* parent) :
    TableForeignKeyPanel(parent)
{
    connect(this, SIGNAL(updateValidation()), this, SIGNAL(modified()));
    connect(ui->onDeleteCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
    connect(ui->onUpdateCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
    connect(ui->matchCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
    connect(ui->onDeleteCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
    connect(ui->onUpdateCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
    connect(ui->matchCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
    connect(ui->deferrableCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
    connect(ui->initiallyCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
}
