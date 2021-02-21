#include "dbcombobox.h"
#include "dblistmodel.h"
#include "db/db.h"

DbComboBox::DbComboBox(QWidget* parent) : QComboBox(parent)
{
    dbComboModel = new DbListModel(this);
    dbComboModel->setCombo(this);
    setModel(dbComboModel);
    setEditable(false);
}

DbListModel* DbComboBox::getModel() const
{
    return dbComboModel;
}

void DbComboBox::setCurrentDb(Db* db)
{
    setCurrentIndex(dbComboModel->getIndexForDb(db));
}

Db* DbComboBox::currentDb() const
{
    return dbComboModel->getDb(currentIndex());
}
