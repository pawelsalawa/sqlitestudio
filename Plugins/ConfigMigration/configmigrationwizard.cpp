#include "configmigrationwizard.h"
#include "ui_configmigrationwizard.h"
#include "configmigration.h"
#include "configmigrationitem.h"
#include "iconmanager.h"
#include "uiutils.h"

ConfigMigrationWizard::ConfigMigrationWizard(QWidget *parent, ConfigMigration* cfgMigration) :
    QWizard(parent),
    ui(new Ui::ConfigMigrationWizard),
    cfgMigration(cfgMigration)
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

    QTreeWidgetItem* treeItem;
    for (ConfigMigrationItem* cfgItem : cfgMigration->getItemsToMigrate())
    {
        treeItem = new QTreeWidgetItem({cfgItem->label});
        treeItem->setData(0, Qt::UserRole, static_cast<int>(cfgItem->type));
        treeItem->setFlags(treeItem->flags() | Qt::ItemIsUserCheckable);
        treeItem->setCheckState(0, Qt::Checked);
        ui->itemsTree->addTopLevelItem(treeItem);
    }
}
