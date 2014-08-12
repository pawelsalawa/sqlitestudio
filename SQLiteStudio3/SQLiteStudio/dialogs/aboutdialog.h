#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
        Q_OBJECT

    public:
        enum InitialMode
        {
            ABOUT,
            LICENSES
        };

        AboutDialog(InitialMode initialMode, QWidget *parent = 0);
        ~AboutDialog();

    private:
        void init(InitialMode initialMode);
        void buildIndex();
        void readLicense(int row, const QString& title, const QString& path);
        QString readFile(const QString& path);

        Ui::AboutDialog *ui;
        QStringList indexContents;
        QString licenseContents;
};

#endif // ABOUTDIALOG_H
