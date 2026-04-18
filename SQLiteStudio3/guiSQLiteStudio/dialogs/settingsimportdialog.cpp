#include "settingsimportdialog.h"
#include "ui_settingsimportdialog.h"
#include "services/config.h"
#include "uiutils.h"
#include <QDialog>
#include <QPushButton>

SettingsImportDialog::SettingsImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsImportDialog)
{
    ui->setupUi(this);

    connect(ui->fileEdit, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

    fileChanged("");
}

SettingsImportDialog::~SettingsImportDialog()
{
    delete ui;
}

void SettingsImportDialog::fileChanged(const QString& filePath)
{
    QString err;
    Config::ExportImportParams params = CFG->getParamsForConfigImport(filePath, &err);

    bool ok = err.isEmpty();
    setValidState(ui->fileEdit, ok, err);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
    ui->appendCheck->setEnabled(ok);
    ui->replaceCheck->setEnabled(ok);

    ui->functionsCheck->setEnabled(params.functions);
    ui->functionsCheck->setChecked(params.functions);

    ui->collationsCheck->setEnabled(params.collations);
    ui->collationsCheck->setChecked(params.collations);

    ui->codeSnippetsCheck->setEnabled(params.snippets);
    ui->codeSnippetsCheck->setChecked(params.snippets);

    ui->extensionsCheck->setEnabled(params.extensions);
    ui->extensionsCheck->setChecked(params.extensions);
}

void SettingsImportDialog::accept()
{
    Config::ExportImportParams params;
    params.functions = ui->functionsCheck->isChecked();
    params.collations = ui->collationsCheck->isChecked();
    params.snippets = ui->codeSnippetsCheck->isChecked();
    params.extensions = ui->extensionsCheck->isChecked();
    params.overwriteOnImport = ui->replaceCheck->isChecked();
    CFG->importConfig(ui->fileEdit->getFile(), params);

    QDialog::accept();
}
