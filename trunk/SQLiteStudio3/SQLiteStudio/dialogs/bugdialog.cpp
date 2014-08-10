#include "bugdialog.h"
#include "ui_bugdialog.h"
#include "iconmanager.h"
#include "uiutils.h"
#include "common/utils.h"
#include "sqlitestudio.h"
#include "services/pluginmanager.h"
#include <QPushButton>
#include <QDebug>

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
    ui->featureRadio->setChecked(true);
}

void BugDialog::init()
{
    ui->setupUi(this);
    resize(width(), height() - 50);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Send"));
    ui->bugRadio->setIcon(ICONS.BUG);
    ui->featureRadio->setIcon(ICONS.FEATURE_REQUEST);

    connect(ui->moreDetailsGroup, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->bugRadio, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->shortDescriptionEdit, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(ui->longDescriptionEdit, SIGNAL(textChanged()), this, SLOT(validate()));

    ui->versionEdit->setText(SQLITESTUDIO->getVersionString());
    ui->osEdit->setText(getOsString());
    ui->pluginsEdit->setText(PLUGINS->getLoadedPluginNames().join(", "));

    updateState();
    validate();
}

void BugDialog::updateState()
{
    ui->scrollArea->setVisible(ui->moreDetailsGroup->isChecked());

    bool bug = ui->bugRadio->isChecked();
    ui->moreDetailsGroup->setVisible(bug);
    if (bug)
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
}

void BugDialog::validate()
{
    int shortSize = ui->shortDescriptionEdit->text().size();
    int longSize = ui->longDescriptionEdit->toPlainText().size();
    bool shortOk = shortSize >= 10 && shortSize <= 100;
    bool longOk = longSize >= 30;

    setValidState(ui->shortDescriptionEdit, shortOk, tr("Short description requires at least 10 characters, but not more than 100. "
                                                        "Longer description can be entered in the field below."));
    setValidState(ui->longDescriptionEdit, longOk, tr("Long description requires at least 30 characters."));

    bool valid = shortOk && longOk;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
