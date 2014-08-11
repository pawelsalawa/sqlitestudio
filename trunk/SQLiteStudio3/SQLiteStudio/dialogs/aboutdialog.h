#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit AboutDialog(QWidget *parent = 0);
        ~AboutDialog();

    private:
        void init();
        void buildIndex();
        void readMainLicense(int row);
        void readIconsLicense(int row);
        QString readFile(const QString& path);

        Ui::AboutDialog *ui;
        QString licenseContents;
};

#endif // ABOUTDIALOG_H
