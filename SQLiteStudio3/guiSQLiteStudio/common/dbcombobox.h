#ifndef DBCOMBOBOX_H
#define DBCOMBOBOX_H

#include <QComboBox>

class QComboBox;
class DbListModel;
class Db;

class DbComboBox : public QComboBox
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
