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
        enum QuickMode
        {
            FUNCTION,
            COLLATION,
            SNIPPET,
            EXTENSION,
        };

        explicit SettingsExportDialog(QWidget *parent = nullptr);
        ~SettingsExportDialog();

        static void exportToFile(QuickMode quickMode);

    private:
        Ui::SettingsExportDialog *ui;

    public slots:
        void accept();
};

#endif // SETTINGSEXPORTDIALOG_H
