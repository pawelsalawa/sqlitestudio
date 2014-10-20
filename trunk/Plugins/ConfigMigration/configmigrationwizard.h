#ifndef CONFIGMIGRATIONWIZARD_H
#define CONFIGMIGRATIONWIZARD_H

#include <QWizard>

namespace Ui {
class ConfigMigrationWizard;
}

class ConfigMigrationWizard : public QWizard
{
        Q_OBJECT

    public:
        explicit ConfigMigrationWizard(QWidget *parent = 0);
        ~ConfigMigrationWizard();

    private:
        void init();

        Ui::ConfigMigrationWizard *ui;
};

#endif // CONFIGMIGRATIONWIZARD_H
