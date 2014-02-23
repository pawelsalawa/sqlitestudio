#ifndef SEARCHTEXTDIALOG_H
#define SEARCHTEXTDIALOG_H

#include <QDialog>

namespace Ui {
    class SearchTextDialog;
}

class SearchTextLocator;

class SearchTextDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit SearchTextDialog(SearchTextLocator* textLocator, QWidget *parent = 0);
        ~SearchTextDialog();

    protected:
        void changeEvent(QEvent *e);
        void showEvent(QShowEvent* e);

    private:
        void applyConfigToLocator();

        Ui::SearchTextDialog *ui;
        SearchTextLocator* textLocator;
        bool configModifiedState = false;

    private slots:
        void setReplaceAvailable(bool available);
        void on_findButton_clicked();
        void on_replaceButton_clicked();
        void on_replaceAllButton_clicked();
};

#endif // SEARCHTEXTDIALOG_H
