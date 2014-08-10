#ifndef BUGDIALOG_H
#define BUGDIALOG_H

#include <QDialog>

namespace Ui {
    class BugDialog;
}

class BugDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BugDialog(QWidget *parent = 0);
        ~BugDialog();

        void setFeatureRequestMode(bool feature);

    private:
        void init();

        Ui::BugDialog *ui;

    private slots:
        void updateState();
        void validate();
};

#endif // BUGDIALOG_H
