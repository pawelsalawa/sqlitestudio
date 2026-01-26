#ifndef DBCOMBOBOX_H
#define DBCOMBOBOX_H

#include "guiSQLiteStudio_global.h"
#include <QComboBox>

class QComboBox;
class DbListModel;
class Db;

class GUI_API_EXPORT DbComboBox : public QComboBox
{
    Q_OBJECT

    public:
        explicit DbComboBox(QWidget* parent = nullptr);

        DbListModel* getModel() const;
        void setCurrentDb(Db* db);
        Db* currentDb() const;

    private:
        DbListModel* dbComboModel = nullptr;

    private slots:
        void handleListCleared();
};

#endif // DBCOMBOBOX_H
