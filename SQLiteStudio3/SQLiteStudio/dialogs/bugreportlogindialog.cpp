#include "bugreportlogindialog.h"
#include "ui_bugreportlogindialog.h"
#include "uiutils.h"
#include "services/bugreporter.h"
#include "iconmanager.h"
#include "common/widgetcover.h"
#include <QPushButton>

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

bool BugReportLoginDialog::isValid() const
{
    return validCredentials;
}

QString BugReportLoginDialog::getLogin() const
{
    return ui->loginEdit->text();
}

QString BugReportLoginDialog::getPassword() const
{
    return ui->passwordEdit->text();
}

void BugReportLoginDialog::init()
{
    ui->setupUi(this);
    connect(ui->loginEdit, SIGNAL(textChanged(QString)), this, SLOT(credentialsChanged()));
    connect(ui->passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(credentialsChanged()));
    connect(ui->validationButton, SIGNAL(clicked()), this, SLOT(remoteValidation()));
    connect(BUGS, SIGNAL(credentialsValidationResult(bool,QString)), this, SLOT(remoteValidationResult(bool,QString)));

    widgetCover = new WidgetCover(this);
    widgetCover->initWithInterruptContainer(tr("Abort"));
    connect(widgetCover, SIGNAL(cancelClicked()), this, SLOT(abortRemoteValidation()));

    validate();
}

void BugReportLoginDialog::credentialsChanged()
{
    validCredentials = false;
    validate();
}

void BugReportLoginDialog::validate()
{
    QString login = ui->loginEdit->text();
    QString pass = ui->passwordEdit->text();

    bool loginOk = login.size() >= 2;
    bool passOk = pass.size() >= 5;

    setValidState(ui->loginEdit, loginOk, tr("A login must be at least 2 characters long."));
    setValidState(ui->passwordEdit, passOk, tr("A password must be at least 5 characters long."));

    bool credentialsOk = loginOk && passOk;
    ui->validationButton->setEnabled(credentialsOk);
    ui->validationLabel->setEnabled(credentialsOk);

    bool valid = credentialsOk && validCredentials;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void BugReportLoginDialog::abortRemoteValidation()
{
    BUGS->abortCredentialsValidation();
}

void BugReportLoginDialog::remoteValidation()
{
    widgetCover->show();
    BUGS->validateBugReportCredentials(ui->loginEdit->text(), ui->passwordEdit->text());
}

void BugReportLoginDialog::remoteValidationResult(bool success, const QString& errorMessage)
{
    validCredentials = success;
    ui->validationButton->setIcon(success ? ICONS.TEST_CONN_OK : ICONS.TEST_CONN_ERROR);
    ui->validationLabel->setText(success ? tr("Valid") : errorMessage);
    validate();
    widgetCover->hide();
}
