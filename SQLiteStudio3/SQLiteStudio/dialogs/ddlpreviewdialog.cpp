#include "ddlpreviewdialog.h"
#include "ui_ddlpreviewdialog.h"
#include "sqlformatter.h"
#include "uiconfig.h"
#include "sqlitestudio.h"

DdlPreviewDialog::DdlPreviewDialog(Dialect dialect, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DdlPreviewDialog),
    dialect(dialect)
{
    ui->setupUi(this);
}

DdlPreviewDialog::~DdlPreviewDialog()
{
    delete ui;
}

void DdlPreviewDialog::setDdl(const QString& ddl)
{
    QString formatted = SQLITESTUDIO->getSqlFormatter()->format(ddl, dialect);
    ui->ddlEdit->setPlainText(formatted);
}

void DdlPreviewDialog::setDdl(const QStringList& ddlList)
{
    QStringList fixedList;
    QString newDdl;
    foreach (const QString& ddl, ddlList)
    {
        newDdl = ddl.trimmed();
        if (!newDdl.endsWith(";"))
            newDdl.append(";");

        fixedList << newDdl;
    }
    setDdl(fixedList.join("\n"));
}

void DdlPreviewDialog::setDdl(QList<SqliteQueryPtr> ddlList)
{
    QStringList list;
    foreach (const SqliteQueryPtr& query, ddlList)
        list << query->detokenize();

    setDdl(list);
}

void DdlPreviewDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void DdlPreviewDialog::accept()
{
    CFG_UI.General.DontShowDdlPreview.set(ui->dontShowAgainCheck->isChecked());
    QDialog::accept();
}
