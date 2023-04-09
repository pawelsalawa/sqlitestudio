#include "configmigration.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
#include "mainwindow.h"
#include "statusfield.h"
#include "configmigrationwizard.h"
#include "db/dbsqlite3.h"
#include "translations.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

ConfigMigration::ConfigMigration()
{
}

bool ConfigMigration::init()
{
    SQLS_INIT_RESOURCE(configmigration);
    loadTranslation("ConfigMigration");

    if (cfg.CfgMigration.Migrated.get())
    {
        qDebug() << "ConfigMigration: already migrated. Skipping.";
        return true;
    }

    QString oldCfg = findOldConfig();
    if (!oldCfg.isNull())
    {
        db = new DbSqlite3("Old SQLiteStudio settings", oldCfg, {{DB_PURE_INIT, true}});
        if (db->open())
        {
            itemsToMigrate = findItemsToMigrate();
            notifyInfo(tr("A configuration from old SQLiteStudio 2.x.x has been detected. "
                          "Would you like to migrate old settings into the current version? "
                          "<a href=\"%1\">Click here to do that</a>.").arg(ACTION_LINK));

            connect(MAINWINDOW->getStatusField(), SIGNAL(linkActivated(QString)), this, SLOT(linkActivated(QString)));
            db->close();
        }
    }

    return true;
}

void ConfigMigration::deinit()
{
    SQLS_CLEANUP_RESOURCE(configmigration);
    safe_delete(db);

    for (ConfigMigrationItem* item : itemsToMigrate)
        delete item;

    itemsToMigrate.clear();
    GenericPlugin::deinit();
}

QString ConfigMigration::findOldConfig()
{
    QString output;
    QString dirPath;

    // Portable path 1 check
    dirPath = QDir::currentPath() + "/sqlitestudio-cfg";
    if (checkOldDir(dirPath, output))
        return output;

    // Portable path 2 check
    dirPath = QCoreApplication::applicationDirPath() + "/sqlitestudio-cfg";
    if (checkOldDir(dirPath, output))
        return output;

    // Portable path 3 check
    dirPath = QCoreApplication::applicationDirPath() + "/../sqlitestudio-cfg";
    if (checkOldDir(dirPath, output))
        return output;

    if (getDistributionType() == DistributionType::OSX_BUNDLE)
    {
        // Portable path 4 check
        dirPath = QCoreApplication::applicationDirPath() + "/../../sqlitestudio-cfg";
        if (checkOldDir(dirPath, output))
            return output;

        // Portable path 5 check
        dirPath = QCoreApplication::applicationDirPath() + "/../../../sqlitestudio-cfg";
        if (checkOldDir(dirPath, output))
            return output;
    }

    // Global path check
#ifdef Q_OS_WIN
    if (QSysInfo::windowsVersion() & QSysInfo::WV_NT_based)
        dirPath = SQLITESTUDIO->getEnv("APPDATA")+"/sqlitestudio";
    else
        dirPath = SQLITESTUDIO->getEnv("HOME")+"/sqlitestudio";
#else
    dirPath = SQLITESTUDIO->getEnv("HOME")+"/.sqlitestudio";
#endif

    if (checkOldDir(dirPath, output))
        return output;

    return QString();
}

bool ConfigMigration::checkOldDir(const QString &dir, QString &output)
{
    QFileInfo fi(dir + "/settings");
    if (fi.exists() && fi.isReadable())
    {
        output = fi.absoluteFilePath();
        return true;
    }

    return false;
}

QList<ConfigMigrationItem*> ConfigMigration::findItemsToMigrate()
{
    static_qstring(bugsHistoryQuery, "SELECT count(*) FROM bugs");
    static_qstring(dbListQuery, "SELECT count(*) FROM dblist");
    static_qstring(funcListQuery, "SELECT count(*) FROM functions");
    static_qstring(sqlHistoryQuery, "SELECT count(*) FROM history");

    ConfigMigrationItem* item = nullptr;
    QList<ConfigMigrationItem*> results;

    int bugReports = db->exec(bugsHistoryQuery)->getSingleCell().toInt();
    if (bugReports > 0)
    {
        item = new ConfigMigrationItem;
        item->type = ConfigMigrationItem::Type::BUG_REPORTS;
        item->label = tr("Bug reports history (%1)").arg(bugReports);
        results << item;
    }

    int dbCount = db->exec(dbListQuery)->getSingleCell().toInt();
    if (dbCount > 0)
    {
        item = new ConfigMigrationItem;
        item->type = ConfigMigrationItem::Type::DATABASES;
        item->label = tr("Database list (%1)").arg(dbCount);
        results << item;
    }

    int funcCount = db->exec(funcListQuery)->getSingleCell().toInt();
    if (funcCount > 0)
    {
        item = new ConfigMigrationItem;
        item->type = ConfigMigrationItem::Type::FUNCTION_LIST;
        item->label = tr("Custom SQL functions (%1)").arg(funcCount);
        results << item;
    }

    int sqlHistory = db->exec(sqlHistoryQuery)->getSingleCell().toInt();
    if (sqlHistory > 0)
    {
        item = new ConfigMigrationItem;
        item->type = ConfigMigrationItem::Type::SQL_HISTORY;
        item->label = tr("SQL queries history (%1)").arg(sqlHistory);
        results << item;
    }

    return results;
}

Db* ConfigMigration::getOldCfgDb() const
{
    return db;
}

QList<ConfigMigrationItem*> ConfigMigration::getItemsToMigrate() const
{
    return itemsToMigrate;
}

void ConfigMigration::linkActivated(const QString &link)
{
    if (link != ACTION_LINK)
        return;

    ConfigMigrationWizard wizard(MAINWINDOW, this);
    wizard.exec();

    if (wizard.didMigrate())
        cfg.CfgMigration.Migrated.set(true);
}
