#include "ddlpreviewdialog.h"
#include "ui_ddlpreviewdialog.h"
#include "services/codeformatter.h"
#include "uiconfig.h"
#include "sqlitestudio.h"
#include "db/db.h"
#include "common/dialogsizehandler.h"

DdlPreviewDialog::DdlPreviewDialog(Db* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DdlPreviewDialog),
    db(db)
{
    ui->setupUi(this);
    DialogSizeHandler::applyFor(this);
}

DdlPreviewDialog::~DdlPreviewDialog()
{
    delete ui;
}

void DdlPreviewDialog::setDdl(const QString& ddl)
{
    QString formatted = SQLITESTUDIO->getCodeFormatter()->format("sql", ddl, db);
    ui->ddlEdit->setPlainText(formatted);
}

void DdlPreviewDialog::setDdl(const QStringList& ddlList)
{
    QStringList fixedList;
    QString newDdl;
    for (const QString& ddl : ddlList)
    {
        newDdl = ddl.trimmed();
        if (!newDdl.endsWith(";"))
            newDdl.append(";");

        fixedList << SQLITESTUDIO->getCodeFormatter()->format("sql", newDdl, db);
    }
    setDdl(fixedList.join("\n"));
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
