#ifndef NEWVERSIONDIALOG_H
#define NEWVERSIONDIALOG_H

#include "services/updatemanager.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class NewVersionDialog;
}

class GUI_API_EXPORT NewVersionDialog : public QDialog
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

        Ui::NewVersionDialog *ui = nullptr;

    private slots:
        void installUpdates();
};

#endif // NEWVERSIONDIALOG_H
