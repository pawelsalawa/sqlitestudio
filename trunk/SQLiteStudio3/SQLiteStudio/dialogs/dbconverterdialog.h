#ifndef DBCONVERTERDIALOG_H
#define DBCONVERTERDIALOG_H

#include <QDialog>

namespace Ui {
    class DbConverterDialog;
}

class DbConverterDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DbConverterDialog(QWidget *parent = 0);
        ~DbConverterDialog();

    private:
        Ui::DbConverterDialog *ui;
};

#endif // DBCONVERTERDIALOG_H
