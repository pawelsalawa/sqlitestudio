#ifndef BUGREPORTER_H
#define BUGREPORTER_H

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <QHash>

class QNetworkAccessManager;
class QNetworkReply;

class BugReporter : public QObject
{
        Q_OBJECT

    public:
        typedef std::function<void(bool success, const QString& errorMessage)> ResponseHandler;

        explicit BugReporter(QObject *parent = 0);

        QUrl getReporterEmailHelpUrl() const;
        QUrl getReporterUserAndPasswordHelpUrl() const;

    private:
        QNetworkAccessManager* networkManager = nullptr;
        QHash<QNetworkReply*,ResponseHandler> replyToHandler;

        static_char* bugReportServiceUrl = "http://sqlitestudio.pl/reportbug3.rvt";
        static_char* reporterEmailHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Reporter_email_address";
        static_char* reporterUserPassHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Reporter_user_and_password";

    signals:

    public slots:
        void reportBug(const QString& email, const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins,
                       ResponseHandler responseHandler = nullptr);
        void reportBug(const QString& user, const QString& passwrod, const QString& title, const QString& details, const QString& version, const QString& os,
                       const QString& plugins, ResponseHandler responseHandler = nullptr);
        void requestFeature(const QString& email, const QString& title, const QString& details, ResponseHandler responseHandler = nullptr);
        void requestFeature(const QString& user, const QString& passwrod, const QString& title, const QString& details, ResponseHandler responseHandler = nullptr);
};

#define BUGS SQLITESTUDIO->getBugReporter()

#endif // BUGREPORTER_H
