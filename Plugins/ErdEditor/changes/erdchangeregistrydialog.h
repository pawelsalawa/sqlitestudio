#ifndef ERDCHANGEREGISTRYDIALOG_H
#define ERDCHANGEREGISTRYDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>

class ErdChangeRegistry;
class Db;
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
        enum RegistryItemRole
        {
            DDL             = QTreeWidgetItem::UserType + 1,
            IS_DDL_CHANGE   = QTreeWidgetItem::UserType + 2
        };

        void populateTree();
        QTreeWidgetItem* createItemFromChange(int& labelIdx, ErdChange* change);

        Ui::ErdChangeRegistryDialog *ui;
        Db* db = nullptr;
        ErdChangeRegistry* changeRegistry = nullptr;

    private slots:
        void handleItemSelected(QTreeWidgetItem* item);
        void compactViewToggled(bool enabled);
        void applyFiltering();
};

#endif // ERDCHANGEREGISTRYDIALOG_H
