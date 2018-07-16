#include "execfromfiledialog.h"
#include "ui_execfromfiledialog.h"
#include "common/utils.h"
#include "uiconfig.h"
#include "uiutils.h"
#include <QFileDialog>

ExecFromFileDialog::ExecFromFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExecFromFileDialog)
{
    init();
}

ExecFromFileDialog::~ExecFromFileDialog()
{
    delete ui;
}

bool ExecFromFileDialog::ignoreErrors() const
{
    return ui->skipErrorsCheck->isChecked();
}

QString ExecFromFileDialog::filePath() const
{
    return ui->fileEdit->text();
}

QString ExecFromFileDialog::codec() const
{
    return ui->encodingCombo->currentText();
}

void ExecFromFileDialog::init()
{
    ui->setupUi(this);

    connect(ui->fileBrowse, SIGNAL(clicked()), this, SLOT(browseForInputFile()));
    connect(ui->fileEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateState()));

    ui->encodingCombo->addItems(textCodecNames());
    ui->encodingCombo->setCurrentText(defaultCodecName());
}

void ExecFromFileDialog::browseForInputFile()
{
    QString dir = getFileDialogInitPath();
    QString filters = tr("SQL scripts (*.sql);;All files (*)");
    QString path = QFileDialog::getOpenFileName(nullptr, tr("Execute SQL file"), dir, filters);
    if (path.isNull())
        return;

    setFileDialogInitPathByFile(path);
    ui->fileEdit->setText(path);
    updateState();
}

void ExecFromFileDialog::updateState()
{
    QString path = ui->fileEdit->text();
    if (path.isEmpty())
    {
        setValidState(ui->fileEdit, false, tr("Please provide file to be executed."));
        return;
    }

    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable())
    {
        setValidState(ui->fileEdit, false, tr("Provided file does not exist or cannot be read."));
        return;
    }

    setValidState(ui->fileEdit, true);
}
