#ifndef CONFIGMIGRATIONWIZARD_H
#define CONFIGMIGRATIONWIZARD_H

#include "configmigrationitem.h"
#include "services/functionmanager.h"
#include <QWizard>

namespace Ui {
class ConfigMigrationWizard;
}

class ConfigMigration;
class Db;

class ConfigMigrationWizard : public QWizard
{
        Q_OBJECT

    public:
        ConfigMigrationWizard(QWidget *parent, ConfigMigration* cfgMigration);
        ~ConfigMigrationWizard();
        bool didMigrate();

    private:
        void init();
        void migrate();
        bool migrateSelected(Db* oldCfgDb, Db* newCfgDb);
        bool migrateBugReports(Db* oldCfgDb, Db* newCfgDb);
        bool migrateDatabases(Db* oldCfgDb, Db* newCfgDb);
        bool migrateFunction(Db* oldCfgDb, Db* newCfgDb);
        bool migrateSqlHistory(Db* oldCfgDb, Db* newCfgDb);
        void finalize();
        void collectCheckedTypes();
        void clearFunctions();

        Ui::ConfigMigrationWizard *ui;
        ConfigMigration* cfgMigration = nullptr;
        QList<ConfigMigrationItem::Type> checkedTypes;
        QList<FunctionManager::ScriptFunction*> fnList;
        bool migrated = false;

    private slots:
        void updateOptions();

    public slots:
        void accept();

    signals:
        void updateOptionsValidation();
};

#endif // CONFIGMIGRATIONWIZARD_H
