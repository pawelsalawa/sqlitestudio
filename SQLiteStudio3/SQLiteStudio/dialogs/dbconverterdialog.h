#ifndef DBCONVERTERDIALOG_H
#define DBCONVERTERDIALOG_H

#include <QDialog>

class DbListModel;
class Db;
class DbVersionConverter;

namespace Ui {
    class DbConverterDialog;
}

class DbConverterDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DbConverterDialog(QWidget *parent = 0);
        ~DbConverterDialog();

        void setDb(Db* db);

    private:
        void init();
        void srcDbChanged();

        Ui::DbConverterDialog *ui;
        DbListModel* dbListModel = nullptr;
        Db* srcDb = nullptr;
        DbVersionConverter* converter = nullptr;

    private slots:
        void srcDbChanged(int index);
};

#endif // DBCONVERTERDIALOG_H
