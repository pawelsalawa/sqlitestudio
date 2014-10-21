#ifndef CONFIGMIGRATIONWIZARD_H
#define CONFIGMIGRATIONWIZARD_H

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

    private:
        void init();
        void migrate();
        bool migrateSelected(Db* oldCfgDb, Db* newCfgDb);
        bool migrateBugReports(Db* oldCfgDb, Db* newCfgDb);

        Ui::ConfigMigrationWizard *ui;
        ConfigMigration* cfgMigration = nullptr;

    public slots:
        void accept();

    signals:
        void updateOptionsValidation();
};

#endif // CONFIGMIGRATIONWIZARD_H
