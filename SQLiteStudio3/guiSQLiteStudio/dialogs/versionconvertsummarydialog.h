#ifndef VERSIONCONVERTSUMMARYDIALOG_H
#define VERSIONCONVERTSUMMARYDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class VersionConvertSummaryDialog;
}

class GUI_API_EXPORT VersionConvertSummaryDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit VersionConvertSummaryDialog(QWidget *parent = 0);
        ~VersionConvertSummaryDialog();

        void setSides(const QList<QPair<QString, QString>>& data);

    protected:
        void showEvent(QShowEvent* e);

    private:
        Ui::VersionConvertSummaryDialog *ui = nullptr;
};

#endif // VERSIONCONVERTSUMMARYDIALOG_H
