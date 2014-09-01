#ifndef NEWVERSIONDIALOG_H
#define NEWVERSIONDIALOG_H

#include "services/updatemanager.h"
#include <QDialog>

namespace Ui {
    class NewVersionDialog;
}

class NewVersionDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit NewVersionDialog(QWidget *parent = 0);
        ~NewVersionDialog();

        void setUpdates(const QList<UpdateManager::UpdateEntry>& updates);

    protected:
        void showEvent(QShowEvent*);

    private:
        void init();

        Ui::NewVersionDialog *ui;
};

#endif // NEWVERSIONDIALOG_H
