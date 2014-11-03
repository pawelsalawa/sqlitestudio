#ifndef BUGREPORTER_H
#define BUGREPORTER_H

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <QHash>

class QNetworkAccessManager;
class QNetworkReply;

class API_EXPORT BugReporter : public QObject
{
        Q_OBJECT

    public:
        typedef std::function<void(bool success, const QString& data)> ResponseHandler;

        explicit BugReporter(QObject *parent = 0);

        QUrl getReporterEmailHelpUrl() const;
        QUrl getReporterUserAndPasswordHelpUrl() const;
        void validateBugReportCredentials(const QString& login, const QString& password);
        void abortCredentialsValidation();
        void useBugReportCredentials(const QString& login, const QString& password);
        void clearBugReportCredentials();

    private:
        void reportBug(const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins,
                       ResponseHandler responseHandler, const QString& urlSuffix);
        void requestFeature(const QString& title, const QString& details, ResponseHandler responseHandler, const QString& urlSuffix);

        static QString escapeParam(const QString& input);
        static QString escapeUrl(const QString& input);

        QNetworkAccessManager* networkManager = nullptr;
        QHash<QNetworkReply*,ResponseHandler> replyToHandler;
        QHash<QNetworkReply*,QPair<bool,QString>> replyToTypeAndTitle;
        QNetworkReply* credentialsValidationInProgress = nullptr;

        static_char* bugReportServiceUrl = "http://sqlitestudio.pl/report_bug3.rvt";
        static_char* reporterEmailHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Reporter_email_address";
        static_char* reporterUserPassHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Reporter_user_and_password";

    signals:
        void credentialsValidationResult(bool success, const QString& errorMessage);

    private slots:
        void finished(QNetworkReply* reply);

    public slots:
        void reportBug(const QString& email, const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins,
                       ResponseHandler responseHandler = nullptr);
        void reportBug(const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins,
                       ResponseHandler responseHandler = nullptr);
        void requestFeature(const QString& email, const QString& title, const QString& details, ResponseHandler responseHandler = nullptr);
        void requestFeature(const QString& title, const QString& details, ResponseHandler responseHandler = nullptr);
};

#define BUGS SQLITESTUDIO->getBugReporter()

#endif // BUGREPORTER_H
