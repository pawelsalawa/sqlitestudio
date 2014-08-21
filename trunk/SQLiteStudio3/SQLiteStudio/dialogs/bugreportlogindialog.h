#ifndef BUGREPORTLOGINDIALOG_H
#define BUGREPORTLOGINDIALOG_H

#include <QDialog>

namespace Ui {
    class BugReportLoginDialog;
}

class BugReportLoginDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BugReportLoginDialog(QWidget *parent = 0);
        ~BugReportLoginDialog();

    private:
        void init();

        Ui::BugReportLoginDialog *ui;
};

#endif // BUGREPORTLOGINDIALOG_H
