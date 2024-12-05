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
#include "services/dbmanager.h"
#include "themetuner.h"

ConfigMigrationWizard::ConfigMigrationWizard(QWidget *parent, ConfigMigration* cfgMigration) :
    QWizard(parent),
    ui(new Ui::ConfigMigrationWizard),
    cfgMigration(cfgMigration)
{
    init();
}

ConfigMigrationWizard::~ConfigMigrationWizard()
{
    clearFunctions();
    delete ui;
}

bool ConfigMigrationWizard::didMigrate()
{
    return migrated;
}

void ConfigMigrationWizard::accept()
{
    migrate();
    QWizard::accept();
}

void ConfigMigrationWizard::init()
{
    ui->setupUi(this);
    THEME_TUNER->darkThemeFix(this);

#ifdef Q_OS_MACX
    resize(width() + 150, height());
    setPixmap(QWizard::BackgroundPixmap, addOpacity(ICONMANAGER->getIcon("config_migration")->pixmap(180, 180), 0.4));
#endif

    ui->optionsPage->setValidator([=, this]() -> bool
    {
        QString grpName = ui->groupNameEdit->text();

        bool grpOk = true;
        QString grpErrorMsg;
        if (ui->dbGroup->isEnabled() && ui->dbGroup->isChecked())
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


    QTreeWidgetItem* treeItem = nullptr;
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
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(updateOptions()));

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
    else
    {
        finalize();
    }
    oldCfgDb->close();
    newCfgDb->close();
    delete newCfgDb;
    clearFunctions();
}

bool ConfigMigrationWizard::migrateSelected(Db* oldCfgDb, Db* newCfgDb)
{
    if (checkedTypes.contains(ConfigMigrationItem::Type::BUG_REPORTS) && !migrateBugReports(oldCfgDb, newCfgDb))
        return false;

    if (checkedTypes.contains(ConfigMigrationItem::Type::DATABASES) && !migrateDatabases(oldCfgDb, newCfgDb))
        return false;

    if (checkedTypes.contains(ConfigMigrationItem::Type::FUNCTION_LIST) && !migrateFunction(oldCfgDb, newCfgDb))
        return false;

    if (checkedTypes.contains(ConfigMigrationItem::Type::SQL_HISTORY) && !migrateSqlHistory(oldCfgDb, newCfgDb))
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

bool ConfigMigrationWizard::migrateDatabases(Db* oldCfgDb, Db* newCfgDb)
{
    static_qstring(oldDbListQuery, "SELECT name, path FROM dblist");
    static_qstring(newDbListInsert, "INSERT INTO dblist (name, path) VALUES (?, ?)");
    static_qstring(groupOrderQuery, "SELECT max([order]) + 1 FROM groups WHERE parent %1");
    static_qstring(groupInsert, "INSERT INTO groups (name, [order], parent, open, dbname) VALUES (?, ?, ?, ?, ?)");

    SqlQueryPtr groupResults;
    SqlQueryPtr insertResults;
    SqlResultsRowPtr row;
    SqlQueryPtr results = oldCfgDb->exec(oldDbListQuery);
    if (results->isError())
    {
        notifyError(tr("Could not read database list from old configuration file in order to migrate it: %1").arg(results->getErrorText()));
        return false;
    }

    // Creating containing group
    bool putInGroup = ui->dbGroup->isEnabled() && ui->dbGroup->isChecked();
    qint64 groupId = -1;
    int order;
    if (putInGroup)
    {
        // Query order
        groupResults = newCfgDb->exec(groupOrderQuery.arg("IS NULL"));
        if (groupResults->isError())
        {
            notifyError(tr("Could not query for available order for containing group in new configuration file in order to migrate the database list: %1")
                        .arg(groupResults->getErrorText()));
            return false;
        }

        order = groupResults->getSingleCell().toInt();

        // Insert group
        groupResults = newCfgDb->exec(groupInsert, {ui->groupNameEdit->text(), order, QVariant(), 1, QVariant()});
        if (groupResults->isError())
        {
            notifyError(tr("Could not create containing group in new configuration file in order to migrate the database list: %1").arg(groupResults->getErrorText()));
            return false;
        }
        groupId = groupResults->getRegularInsertRowId();
    }

    // Migrating the list
    QString name;
    QString path;
    while (results->hasNext())
    {
        row = results->next();
        name = row->value("name").toString();
        path = row->value("path").toString();

        if (DBLIST->getByName(name) || DBLIST->getByPath(path)) // already on the new list
            continue;

        insertResults = newCfgDb->exec(newDbListInsert, {name, path});
        if (insertResults->isError())
        {
            notifyError(tr("Could not insert a database entry into new configuration file: %1").arg(insertResults->getErrorText()));
            return false;
        }

        // Query order
        if (putInGroup)
            groupResults = newCfgDb->exec(groupOrderQuery.arg("= ?"), {groupId});
        else
            groupResults = newCfgDb->exec(groupOrderQuery.arg("IS NULL"));

        if (groupResults->isError())
        {
            notifyError(tr("Could not query for available order for next database in new configuration file in order to migrate the database list: %1")
                        .arg(groupResults->getErrorText()));
            return false;
        }

        order = groupResults->getSingleCell().toInt();

        // Insert group
        groupResults = newCfgDb->exec(groupInsert, {QVariant(), order, putInGroup ? QVariant(groupId) : QVariant(), 0, name});
        if (groupResults->isError())
        {
            notifyError(tr("Could not create group referencing the database in new configuration file: %1").arg(groupResults->getErrorText()));
            return false;
        }
    }

    return true;
}

bool ConfigMigrationWizard::migrateFunction(Db* oldCfgDb, Db* newCfgDb)
{
    UNUSED(newCfgDb);

    static_qstring(oldFunctionsQuery, "SELECT name, type, code FROM functions");

    SqlResultsRowPtr row;
    SqlQueryPtr results = oldCfgDb->exec(oldFunctionsQuery);
    if (results->isError())
    {
        notifyError(tr("Could not read function list from old configuration file in order to migrate it: %1").arg(results->getErrorText()));
        return false;
    }

    clearFunctions();
    for (FunctionManager::ScriptFunction* fn : FUNCTIONS->getAllScriptFunctions())
        fnList << new FunctionManager::ScriptFunction(*fn);

    FunctionManager::ScriptFunction* fn = nullptr;
    while (results->hasNext())
    {
        row = results->next();

        fn = new FunctionManager::ScriptFunction();
        fn->type = FunctionManager::ScriptFunction::SCALAR;
        fn->lang = row->value("type").toString();
        fn->name = row->value("name").toString();
        fn->code = row->value("code").toString();
        fnList << fn;
    }

    return true;
}

bool ConfigMigrationWizard::migrateSqlHistory(Db* oldCfgDb, Db* newCfgDb)
{
    static_qstring(historyIdQuery, "SELECT CASE WHEN max(id) IS NULL THEN 0 ELSE max(id) + 1 END FROM sqleditor_history");
    static_qstring(oldHistoryQuery, "SELECT dbname, date, time, rows, sql FROM history");
    static_qstring(newHistoryInsert, "INSERT INTO sqleditor_history (id, dbname, date, time_spent, rows, sql) VALUES (?, ?, ?, ?, ?, ?)");

    SqlQueryPtr insertResults;
    SqlResultsRowPtr row;
    SqlQueryPtr results = oldCfgDb->exec(oldHistoryQuery);
    if (results->isError())
    {
        notifyError(tr("Could not read SQL queries history from old configuration file in order to migrate it: %1").arg(results->getErrorText()));
        return false;
    }

    SqlQueryPtr idResults = newCfgDb->exec(historyIdQuery);
    if (idResults->isError())
    {
        notifyError(tr("Could not read next ID for SQL queries history in new configuration file: %1").arg(idResults->getErrorText()));
        return false;
    }
    qint64 nextId = idResults->getSingleCell().toLongLong();

    int timeSpent;
    int date;
    while (results->hasNext())
    {
        row = results->next();
        timeSpent = qRound(row->value("time").toDouble() * 1000);
        date = QDateTime::fromString(row->value("date").toString(), "yyyy-MM-dd HH:mm").toSecsSinceEpoch();

        insertResults = newCfgDb->exec(newHistoryInsert, {nextId++, row->value("dbname"), date, timeSpent, row->value("rows"), row->value("sql")});
        if (insertResults->isError())
        {
            notifyError(tr("Could not insert SQL history entry into new configuration file: %1").arg(insertResults->getErrorText()));
            return false;
        }
    }

    return true;
}

void ConfigMigrationWizard::finalize()
{
    if (checkedTypes.contains(ConfigMigrationItem::Type::FUNCTION_LIST))
    {
        FUNCTIONS->setScriptFunctions(fnList);
        fnList.clear();
    }

    if (checkedTypes.contains(ConfigMigrationItem::Type::SQL_HISTORY))
        CFG->refreshSqlHistory();

    if (checkedTypes.contains(ConfigMigrationItem::Type::DATABASES))
    {
        bool ignore = DBTREE->getModel()->getIgnoreDbLoadedSignal();
        DBTREE->getModel()->setIgnoreDbLoadedSignal(true);
        DBLIST->scanForNewDatabasesInConfig();
        DBTREE->getModel()->setIgnoreDbLoadedSignal(ignore);
        DBTREE->getModel()->loadDbList();
    }

    migrated = true;
}

void ConfigMigrationWizard::collectCheckedTypes()
{
    checkedTypes.clear();

    QTreeWidgetItem* item = nullptr;
    for (int i = 0, total = ui->itemsTree->topLevelItemCount(); i < total; ++i)
    {
        item = ui->itemsTree->topLevelItem(i);
        if (item->checkState(0) != Qt::Checked)
            continue;

        checkedTypes << static_cast<ConfigMigrationItem::Type>(item->data(0, Qt::UserRole).toInt());
    }
}

void ConfigMigrationWizard::clearFunctions()
{
    for (FunctionManager::ScriptFunction* fn : fnList)
        delete fn;

    fnList.clear();
}

void ConfigMigrationWizard::updateOptions()
{
    if (currentPage() == ui->optionsPage)
    {
        collectCheckedTypes();
        ui->dbGroup->setEnabled(checkedTypes.contains(ConfigMigrationItem::Type::DATABASES));
    }
}
