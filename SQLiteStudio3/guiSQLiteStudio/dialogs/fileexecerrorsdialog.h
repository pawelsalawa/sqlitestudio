#ifndef FILEEXECERRORSDIALOG_H
#define FILEEXECERRORSDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class FileExecErrorsDialog;
}

class QTableWidgetItem;

class GUI_API_EXPORT FileExecErrorsDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit FileExecErrorsDialog(const QList<QPair<QString, QString> >& errors, bool rolledBack, QWidget *parent = nullptr);
        ~FileExecErrorsDialog();

    private:
        QTableWidgetItem* item(const QString& text);

        Ui::FileExecErrorsDialog *ui;
};

#endif // FILEEXECERRORSDIALOG_H
