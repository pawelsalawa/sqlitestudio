#include "bugdialog.h"
#include "ui_bugdialog.h"
#include "iconmanager.h"
#include "uiutils.h"
#include "common/utils.h"
#include "sqlitestudio.h"
#include "mainwindow.h"
#include "services/pluginmanager.h"
#include "services/bugreporter.h"
#include "services/notifymanager.h"
#include <QPushButton>
#include <QDebug>
#include <QDesktopServices>

BugDialog::BugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BugDialog)
{
    init();
}

BugDialog::~BugDialog()
{
    delete ui;
}

void BugDialog::setFeatureRequestMode(bool feature)
{
    bugMode = !feature;
}

void BugDialog::init()
{
    ui->setupUi(this);
    resize(width(), height() - 50);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Send"));

    connect(ui->moreDetailsGroup, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->shortDescriptionEdit, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(ui->longDescriptionEdit, SIGNAL(textChanged()), this, SLOT(validate()));
    connect(ui->emailEdit, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(ui->helpButton, SIGNAL(clicked()), this, SLOT(help()));

    ui->versionEdit->setText(SQLITESTUDIO->getVersionString());
    ui->osEdit->setText(getOsString());
    ui->pluginsEdit->setText(PLUGINS->getLoadedPluginNames().join(", "));

    updateState();
    validate();
}

QString BugDialog::getMessageAboutReportHistory()
{
    return tr("You can see all your reported bugs and ideas by selecting menu '%1' and then '%2'.").arg(MAINWINDOW->getSQLiteStudioMenu()->title())
            .arg(MAINWINDOW->getAction(MainWindow::BUG_REPORT_HISTORY)->text());
}

void BugDialog::finishedBugReport(int code, const QString& errorMsg)
{
    if (code == 0)
    {
        notifyInfo(tr("A bug report sent successfly.") + " " + getMessageAboutReportHistory());
    }
    else
    {
        qCritical() << "Report bug error. Code:" << code << ", msg:" << errorMsg;
        notifyError(tr("An error occurred while sending a bug report: %1").arg(errorMsg));
    }
}

void BugDialog::finishedFeatureRequest(int code, const QString& errorMsg)
{
    if (code == 0)
    {
        notifyInfo(tr("An idea proposal sent successfly.") + " " + getMessageAboutReportHistory());
    }
    else
    {
        qCritical() << "Report feature request error. Code:" << code << ", msg:" << errorMsg;
        notifyError(tr("An error occurred while sending an idea proposal: %1").arg(errorMsg));
    }
}

void BugDialog::updateState()
{
    ui->scrollArea->setVisible(ui->moreDetailsGroup->isChecked());

    ui->moreDetailsGroup->setVisible(bugMode);
    if (bugMode)
    {
        setWindowTitle(tr("A bug report"));
        ui->shortDescriptionEdit->setPlaceholderText(tr("Describe problem in few words"));
        ui->longDescriptionEdit->setPlaceholderText(tr("Describe problem and how to reproduce it"));
    }
    else
    {
        setWindowTitle(tr("A new feature idea"));
        ui->shortDescriptionEdit->setPlaceholderText(tr("A title for your idea"));
        ui->longDescriptionEdit->setPlaceholderText(tr("Describe your idea in more details"));
    }

    if (user.isNull())
    {
        ui->currentLoginLabel->setToolTip(tr("Reporting as an unregistered user, using e-mail address."));
        ui->currentLoginLabel->setPixmap(ICONS.USER_UNKNOWN);
        ui->emailEdit->setEnabled(true);
        ui->emailEdit->clear();
        ui->loginButton->setText(tr("Log in"));
        ui->loginButton->setIcon(ICONS.USER);
    }
    else
    {
        ui->currentLoginLabel->setToolTip(tr("Reporting as a registered user."));
        ui->currentLoginLabel->setPixmap(ICONS.USER);
        ui->emailEdit->setText(user);
        ui->emailEdit->setEnabled(false);
        ui->loginButton->setText(tr("Log out"));
        ui->loginButton->setIcon(ICONS.USER_UNKNOWN);
    }
}

void BugDialog::validate()
{
    bool emailOk = validateEmail(ui->emailEdit->text());
    int shortSize = ui->shortDescriptionEdit->text().trimmed().size();
    int longSize = ui->longDescriptionEdit->toPlainText().trimmed().size();
    bool shortOk = shortSize >= 10 && shortSize <= 100;
    bool longOk = longSize >= 30;

    setValidStateWihtTooltip(ui->emailEdit, tr("Providing true email address will make it possible to contact you regarding your report. "
                                               "To learn more, press 'help' button on the right side."),
                             emailOk, tr("Enter vaild e-mail address."));

    setValidState(ui->shortDescriptionEdit, shortOk, tr("Short description requires at least 10 characters, but not more than 100. "
                                                        "Longer description can be entered in the field below."));

    setValidState(ui->longDescriptionEdit, longOk, tr("Long description requires at least 30 characters."));

    bool valid = shortOk && longOk && emailOk;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void BugDialog::help()
{
    QDesktopServices::openUrl(QUrl(BUGS->getReporterEmailHelpUrl()));
}

void BugDialog::accept()
{
    if (bugMode)
    {
        if (user.isNull())
        {
            BUGS->reportBug(ui->emailEdit->text(), ui->shortDescriptionEdit->text(), ui->longDescriptionEdit->toPlainText(), ui->versionEdit->text(),
                            ui->osEdit->text(), ui->pluginsEdit->text(), BugDialog::finishedBugReport);
        }
        else
        {
            BUGS->reportBug(CFG_CORE.General.BugReportUser.get(), CFG_CORE.General.BugReportPassword.get(), ui->shortDescriptionEdit->text(),
                            ui->longDescriptionEdit->toPlainText(), ui->versionEdit->text(), ui->osEdit->text(), ui->pluginsEdit->text(),
                            BugDialog::finishedFeatureRequest);
        }
    }
    else
    {
        if (user.isNull())
        {
            BUGS->requestFeature(ui->emailEdit->text(), ui->shortDescriptionEdit->text(), ui->longDescriptionEdit->toPlainText(), BugDialog::finishedFeatureRequest);
        }
        else
        {
            BUGS->requestFeature(CFG_CORE.General.BugReportUser.get(), CFG_CORE.General.BugReportPassword.get(), ui->shortDescriptionEdit->text(),
                                 ui->longDescriptionEdit->toPlainText(), BugDialog::finishedFeatureRequest);
        }
    }
    QDialog::accept();
}
