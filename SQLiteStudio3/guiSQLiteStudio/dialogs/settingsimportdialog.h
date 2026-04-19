#ifndef SETTINGSIMPORTDIALOG_H
#define SETTINGSIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
    class SettingsImportDialog;
}

class SettingsImportDialog : public QDialog
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

        explicit SettingsImportDialog(QWidget *parent = nullptr);
        ~SettingsImportDialog();

        static void importFromFile(QuickMode quickMode);

    private:
        Ui::SettingsImportDialog *ui;

    private slots:
        void fileChanged(const QString& filePath);

    public slots:
        void accept();
};

#endif // SETTINGSIMPORTDIALOG_H
