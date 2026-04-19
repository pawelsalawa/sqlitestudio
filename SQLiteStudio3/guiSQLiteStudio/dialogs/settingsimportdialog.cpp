#include "settingsimportdialog.h"
#include "services/notifymanager.h"
#include "ui_settingsimportdialog.h"
#include "services/config.h"
#include "uiconfig.h"
#include "uiutils.h"
#include "mainwindow.h"
#include <QDialog>
#include <QFileDialog>
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

void SettingsImportDialog::importFromFile(QuickMode quickMode)
{
    QString dir = getFileDialogInitPath();
    QString path = QFileDialog::getOpenFileName(MAINWINDOW, tr("Input JSON file"), dir, "JSON file (*.json);;All files (*)");

    if (path.isEmpty())
        return;

    setFileDialogInitPathByFile(path);

    QString err;
    Config::ExportImportParams queriedParams = CFG->getParamsForConfigImport(path, &err);
    if (!err.isEmpty())
    {
        notifyWarn(tr("Invalid input file to import: %1").arg(err));
        return;
    }

    Config::ExportImportParams params = {false, false, false, false, false};
    switch (quickMode)
    {
        case FUNCTION:
        {
            if (!queriedParams.functions)
            {
                notifyWarn(tr("Selected file does not contain functions to import."));
                return;
            }
            params.functions = true;
            break;
        }
        case COLLATION:
            if (!queriedParams.collations)
            {
                notifyWarn(tr("Selected file does not contain collation sequences to import."));
                return;
            }
            params.collations = true;
            break;
        case SNIPPET:
            if (!queriedParams.snippets)
            {
                notifyWarn(tr("Selected file does not contain code snippets to import."));
                return;
            }
            params.snippets = true;
            break;
        case EXTENSION:
            if (!queriedParams.extensions)
            {
                notifyWarn(tr("Selected file does not contain SQLite extensions to import."));
                return;
            }
            params.extensions = true;
            break;
    }

    CFG->importConfig(path, params);
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
