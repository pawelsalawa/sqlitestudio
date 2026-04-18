#include "settingsexportdialog.h"
#include "ui_settingsexportdialog.h"
#include <QFile>
#include <QFileDialog>
#include "services/config.h"

SettingsExportDialog::SettingsExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsExportDialog)
{
    ui->setupUi(this);
}

SettingsExportDialog::~SettingsExportDialog()
{
    delete ui;
}

void SettingsExportDialog::accept()
{
    Config::ExportImportParams params;
    params.functions = ui->functionsCheck->isChecked();
    params.collations = ui->collationsCheck->isChecked();
    params.snippets = ui->codeSnippetsCheck->isChecked();
    params.extensions = ui->extensionsCheck->isChecked();
    CFG->exportConfig(ui->fileEdit->getFile(), params);

    QDialog::accept();
}
