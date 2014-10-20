#include "configmigrationwizard.h"
#include "ui_configmigrationwizard.h"
#include "iconmanager.h"
#include "uiutils.h"

ConfigMigrationWizard::ConfigMigrationWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::ConfigMigrationWizard)
{
    init();
}

ConfigMigrationWizard::~ConfigMigrationWizard()
{
    delete ui;
}

void ConfigMigrationWizard::init()
{
    ui->setupUi(this);

#ifdef Q_OS_MACX
    resize(width() + 150, height());
    setPixmap(QWizard::BackgroundPixmap, addOpacity(ICONMANAGER->getIcon("config_migration")->pixmap(180, 180), 0.4));
#endif
}
