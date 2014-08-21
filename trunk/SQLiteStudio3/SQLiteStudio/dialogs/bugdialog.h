#ifndef BUGDIALOG_H
#define BUGDIALOG_H

#include <QDialog>

namespace Ui {
    class BugDialog;
}

class BugDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BugDialog(QWidget *parent = 0);
        ~BugDialog();

        void setFeatureRequestMode(bool feature);

    private:
        void init();

        static QString getMessageAboutReportHistory();
        static void finishedBugReport(int code, const QString& errorMsg);
        static void finishedFeatureRequest(int code, const QString& errorMsg);

        Ui::BugDialog *ui;
        bool bugMode = true;
        QString user;

    private slots:
        void updateState();
        void validate();
        void help();

    public slots:
        void accept();
};

#endif // BUGDIALOG_H
