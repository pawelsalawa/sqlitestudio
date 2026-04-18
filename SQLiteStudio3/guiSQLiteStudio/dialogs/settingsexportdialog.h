#ifndef SETTINGSEXPORTDIALOG_H
#define SETTINGSEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
    class SettingsExportDialog;
}

class SettingsExportDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit SettingsExportDialog(QWidget *parent = nullptr);
        ~SettingsExportDialog();

    private:
        Ui::SettingsExportDialog *ui;

    public slots:
        void accept();
};

#endif // SETTINGSEXPORTDIALOG_H
