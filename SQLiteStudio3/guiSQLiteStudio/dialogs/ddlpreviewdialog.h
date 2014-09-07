#ifndef DDLPREVIEWDIALOG_H
#define DDLPREVIEWDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

class Db;

namespace Ui {
    class DdlPreviewDialog;
}

class GUI_API_EXPORT DdlPreviewDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DdlPreviewDialog(Db* db, QWidget *parent = 0);
        ~DdlPreviewDialog();

        void setDdl(const QString& ddl);
        void setDdl(const QStringList& ddlList);

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::DdlPreviewDialog *ui;
        Db* db = nullptr;

    public slots:
        void accept();
};

#endif // DDLPREVIEWDIALOG_H
