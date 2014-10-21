#include "configmigrationwizard.h"
#include "ui_configmigrationwizard.h"
#include "configmigration.h"
#include "configmigrationitem.h"
#include "iconmanager.h"
#include "uiutils.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "services/config.h"
#include "sqlitestudio.h"
#include "db/dbsqlite3.h"
#include "services/notifymanager.h"

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

void ConfigMigrationWizard::accept()
{
    migrate();
    QWizard::accept();
}

void ConfigMigrationWizard::init()
{
    ui->setupUi(this);

#ifdef Q_OS_MACX
    resize(width() + 150, height());
    setPixmap(QWizard::BackgroundPixmap, addOpacity(ICONMANAGER->getIcon("config_migration")->pixmap(180, 180), 0.4));
#endif

    ui->optionsPage->setValidator([=]() -> bool
    {
        QString grpName = ui->groupNameEdit->text();

        bool grpOk = true;
        QString grpErrorMsg;
        if (ui->dbGroup->isChecked())
        {
            if (grpName.isEmpty())
            {
                grpOk = false;
                grpErrorMsg = tr("Enter a non-empty name.");
            }
            else
            {
                DbTreeItem* item = DBTREE->getModel()->findItem(DbTreeItem::Type::DIR, grpName);
                if (item && !item->parentDbTreeItem())
                {
                    grpOk = false;
                    grpErrorMsg = tr("Top level group named '%1' already exists. Enter a group name that does not exist yet.").arg(grpName);
                }
            }
        }

        setValidState(ui->groupNameEdit, grpOk, grpErrorMsg);

        return grpOk;
    });


    QTreeWidgetItem* treeItem;
    for (ConfigMigrationItem* cfgItem : cfgMigration->getItemsToMigrate())
    {
        treeItem = new QTreeWidgetItem({cfgItem->label});
        treeItem->setData(0, Qt::UserRole, static_cast<int>(cfgItem->type));
        treeItem->setFlags(treeItem->flags() | Qt::ItemIsUserCheckable);
        treeItem->setCheckState(0, Qt::Checked);
        ui->itemsTree->addTopLevelItem(treeItem);
    }

    connect(ui->dbGroup, SIGNAL(clicked()), ui->optionsPage, SIGNAL(completeChanged()));
    connect(ui->groupNameEdit, SIGNAL(textChanged(QString)), ui->optionsPage, SIGNAL(completeChanged()));
    connect(this, SIGNAL(updateOptionsValidation()), ui->optionsPage, SIGNAL(completeChanged()));

    emit updateOptionsValidation();
}

void ConfigMigrationWizard::migrate()
{
    Db* oldCfgDb = cfgMigration->getOldCfgDb();
    if (!oldCfgDb->open())
    {
        notifyError(tr("Could not open old configuration file in order to migrate settings from it."));
        return;
    }

    QString cfgFilePath = SQLITESTUDIO->getConfig()->getConfigFilePath();
    Db* newCfgDb = new DbSqlite3("Config migration connection", cfgFilePath, {{DB_PURE_INIT, true}});
    if (!newCfgDb->open())
    {
        notifyError(tr("Could not open current configuration file in order to migrate settings from old configuration file."));
        delete newCfgDb;
        return;
    }

    newCfgDb->begin();
    bool migrated = migrateSelected(oldCfgDb, newCfgDb);
    if (migrated && !newCfgDb->commit())
    {
        notifyError(tr("Could not commit migrated data into new configuration file: %1").arg(newCfgDb->getErrorText()));
        newCfgDb->rollback();
    }
    else if (!migrated)
    {
        newCfgDb->rollback();
    }
    oldCfgDb->close();
    newCfgDb->close();
    delete newCfgDb;
}

bool ConfigMigrationWizard::migrateSelected(Db* oldCfgDb, Db* newCfgDb)
{
    if (!migrateBugReports(oldCfgDb, newCfgDb))
        return false;

    return true;
}

bool ConfigMigrationWizard::migrateBugReports(Db* oldCfgDb, Db* newCfgDb)
{
    static_qstring(oldBugsQuery, "SELECT created_on, brief, url, type FROM bugs");
    static_qstring(newBugsInsert, "INSERT INTO reports_history (timestamp, feature_request, title, url) VALUES (?, ?, ?, ?)");

    SqlQueryPtr insertResults;
    SqlResultsRowPtr row;
    SqlQueryPtr results = oldCfgDb->exec(oldBugsQuery);
    if (results->isError())
    {
        notifyError(tr("Could not read bug reports history from old configuration file in order to migrate it: %1").arg(results->getErrorText()));
        return false;
    }

    bool feature;
    QString url;
    while (results->hasNext())
    {
        row = results->next();
        feature = (row->value("type").toString().toUpper() == "FEATURE");
        url = row->value("url").toString().trimmed();
        if (url.startsWith("http://") && url.contains("sqlitestudio.one.pl"))
            url.replace("sqlitestudio.one.pl", "sqlitestudio.pl").replace("report_bug.rvt", "report_bug3.rvt");

        insertResults = newCfgDb->exec(newBugsInsert, {row->value("created_on"), feature, row->value("brief"), url});
        if (insertResults->isError())
        {
            notifyError(tr("Could not insert a bug reports history entry into new configuration file: %1").arg(insertResults->getErrorText()));
            return false;
        }
    }

    return true;
}
