#ifndef TRIGGERCOLUMNSDIALOG_H
#define TRIGGERCOLUMNSDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class TriggerColumnsDialog;
}

class QCheckBox;

class GUI_API_EXPORT TriggerColumnsDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit TriggerColumnsDialog(QWidget *parent, int globalX, int globalY);
        ~TriggerColumnsDialog();

        void addColumn(const QString& name, bool checked);
        QStringList getCheckedColumns() const;

    protected:
        void changeEvent(QEvent *e);
        void showEvent(QShowEvent*);

    private:
        QList<QCheckBox*> checkBoxList;
        int globalX;
        int globalY;
        Ui::TriggerColumnsDialog *ui = nullptr;
};

#endif // TRIGGERCOLUMNSDIALOG_H
