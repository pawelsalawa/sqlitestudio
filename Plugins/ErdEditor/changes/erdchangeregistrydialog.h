#ifndef ERDCHANGEREGISTRYDIALOG_H
#define ERDCHANGEREGISTRYDIALOG_H

#include <QDialog>

class ErdChangeRegistry;
class Db;
class QTreeWidgetItem;
class ErdChange;

namespace Ui {
    class ErdChangeRegistryDialog;
}

class ErdChangeRegistryDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit ErdChangeRegistryDialog(Db* db, ErdChangeRegistry* changeRegistry, QWidget *parent = nullptr);
        ~ErdChangeRegistryDialog();

    private:
        void populateTree();
        QTreeWidgetItem* createItemFromChange(int rowIdx, ErdChange* change);

        Ui::ErdChangeRegistryDialog *ui;
        Db* db = nullptr;
        ErdChangeRegistry* changeRegistry = nullptr;

    private slots:
        void handleItemSelected(QTreeWidgetItem* item);
};

#endif // ERDCHANGEREGISTRYDIALOG_H
