#include "settingsexportdialog.h"
#include "ui_settingsexportdialog.h"
#include "mainwindow.h"
#include "services/config.h"
#include "uiconfig.h"
#include <QFile>
#include <QFileDialog>

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

void SettingsExportDialog::exportToFile(QuickMode quickMode)
{
    QString dir = getFileDialogInitPath();
    QString path = QFileDialog::getOpenFileName(MAINWINDOW, tr("Output JSON file"), dir, "JSON file (*.json);;All files (*)");

    if (path.isEmpty())
        return;

    setFileDialogInitPathByFile(path);

    Config::ExportImportParams params = {false, false, false, false, false};
    switch (quickMode)
    {
        case FUNCTION:
        {
            params.functions = true;
            break;
        }
        case COLLATION:
            params.collations = true;
            break;
        case SNIPPET:
            params.snippets = true;
            break;
        case EXTENSION:
            params.extensions = true;
            break;
    }
    CFG->exportConfig(path, params);

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
