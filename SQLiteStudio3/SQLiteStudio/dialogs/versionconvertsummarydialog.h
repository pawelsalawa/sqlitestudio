#ifndef VERSIONCONVERTSUMMARYDIALOG_H
#define VERSIONCONVERTSUMMARYDIALOG_H

#include <QDialog>

namespace Ui {
    class VersionConvertSummaryDialog;
}

class VersionConvertSummaryDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit VersionConvertSummaryDialog(QWidget *parent = 0);
        ~VersionConvertSummaryDialog();

        void setSides(const QList<QPair<QString, QString>>& data);

    private:
        Ui::VersionConvertSummaryDialog *ui;
};

#endif // VERSIONCONVERTSUMMARYDIALOG_H
