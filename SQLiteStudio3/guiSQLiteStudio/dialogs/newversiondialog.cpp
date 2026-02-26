#ifdef HAS_UPDATEMANAGER

#include "newversiondialog.h"
#include "services/pluginmanager.h"
#include "sqlitestudio.h"
#include "ui_newversiondialog.h"
#include "services/config.h"
#include <QDesktopServices>
#include <QInputDialog>
#include <QUrl>

NewVersionDialog::NewVersionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewVersionDialog)
{
    init();
}

NewVersionDialog::~NewVersionDialog()
{
    delete ui;
}

void NewVersionDialog::setUpdate(const QString& version, const QString& url)
{
    downloadUrl = url;
    ui->versionLabel->setText(version);
}

void NewVersionDialog::init()
{
    ui->setupUi(this);

    connect(ui->abortButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(downloadUpdates()));
    connect(ui->homepageButton, SIGNAL(clicked()), this, SLOT(openHomePage()));
    connect(ui->checkOnStartupCheck, &QCheckBox::clicked, [=](bool checked)
    {
        CFG_CORE.General.CheckUpdatesOnStartup.set(checked);
    });
}

void NewVersionDialog::downloadUpdates()
{
    QDesktopServices::openUrl(QUrl(downloadUrl));
    close();
}

void NewVersionDialog::openHomePage()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getHomePage()));
    close();
}

void NewVersionDialog::showEvent(QShowEvent*)
{
    ui->checkOnStartupCheck->setChecked(CFG_CORE.General.CheckUpdatesOnStartup.get());
}

#endif // HAS_UPDATEMANAGER
