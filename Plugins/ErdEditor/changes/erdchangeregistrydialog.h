#ifndef ERDCHANGEREGISTRYDIALOG_H
#define ERDCHANGEREGISTRYDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "changes/erdeffectivechange.h"
#include "erdeffectivechangemerger.h"

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
        explicit ErdChangeRegistryDialog(Db* db, ErdChangeRegistry* changeRegistry, const QStringList& schemaBase,
                                         QWidget *parent = nullptr);
        ~ErdChangeRegistryDialog();

    private:
        enum RegistryItemRole
        {
            DDL             = QTreeWidgetItem::UserType + 1,
            IS_DDL_CHANGE   = QTreeWidgetItem::UserType + 2
        };

        void populateTree();
        void populateEffectiveChanges();
        void selectFirstVisibleItem();
        QTreeWidgetItem* createItemFromChange(int& labelIdx, ErdChange* change);
        QTreeWidgetItem* createItemFromChange(int& labelIdx, const ErdEffectiveChange& change,
                                              const ErdEffectiveChangeMerger& merger);

        Ui::ErdChangeRegistryDialog *ui;
        Db* db = nullptr;
        ErdChangeRegistry* changeRegistry = nullptr;
        QStringList schemaBase;

    private slots:
        void handleItemSelected(QTreeWidgetItem* item);
        void compactViewToggled(bool enabled);
        void applyFiltering();
};

#endif // ERDCHANGEREGISTRYDIALOG_H
