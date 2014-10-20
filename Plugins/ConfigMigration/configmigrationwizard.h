#ifndef CONFIGMIGRATIONWIZARD_H
#define CONFIGMIGRATIONWIZARD_H

#include <QWizard>

namespace Ui {
class ConfigMigrationWizard;
}

class ConfigMigration;

class ConfigMigrationWizard : public QWizard
{
        Q_OBJECT

    public:
        ConfigMigrationWizard(QWidget *parent, ConfigMigration* cfgMigration);
        ~ConfigMigrationWizard();

    private:
        void init();

        Ui::ConfigMigrationWizard *ui;
        ConfigMigration* cfgMigration = nullptr;
};

#endif // CONFIGMIGRATIONWIZARD_H
