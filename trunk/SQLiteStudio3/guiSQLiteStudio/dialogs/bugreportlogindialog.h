#ifndef BUGREPORTLOGINDIALOG_H
#define BUGREPORTLOGINDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class BugReportLoginDialog;
}

class WidgetCover;

class GUI_API_EXPORT BugReportLoginDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BugReportLoginDialog(QWidget *parent = 0);
        ~BugReportLoginDialog();

        bool isValid() const;
        QString getLogin() const;
        QString getPassword() const;

    private:
        void init();

        Ui::BugReportLoginDialog *ui;
        bool validCredentials = false;
        WidgetCover* widgetCover = nullptr;

    private slots:
        void credentialsChanged();
        void validate();
        void abortRemoteValidation();
        void remoteValidation();
        void remoteValidationResult(bool success, const QString& errorMessage);
};

#endif // BUGREPORTLOGINDIALOG_H
