#ifndef DDLPREVIEWDIALOG_H
#define DDLPREVIEWDIALOG_H

#include "parser/ast/sqlitequery.h"
#include <QDialog>

namespace Ui {
    class DdlPreviewDialog;
}

class DdlPreviewDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DdlPreviewDialog(Dialect dialect, QWidget *parent = 0);
        ~DdlPreviewDialog();

        void setDdl(const QString& ddl);
        void setDdl(const QStringList& ddlList);
        void setDdl(QList<SqliteQueryPtr> ddlList);

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::DdlPreviewDialog *ui;
        Dialect dialect;

    public slots:
        void accept();
};

#endif // DDLPREVIEWDIALOG_H
