#include "bugreporter.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

BugReporter::BugReporter(QObject *parent) :
    QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
}

QUrl BugReporter::getReporterEmailHelpUrl() const
{
    return QUrl(QString::fromLatin1(reporterEmailHelpUrl));
}

QUrl BugReporter::getReporterUserAndPasswordHelpUrl() const
{
    return QUrl(QString::fromLatin1(reporterUserPassHelpUrl));
}

void BugReporter::reportBug(const QString& email, const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins,
                            ResponseHandler responseHandler)
{

}

void BugReporter::reportBug(const QString& user, const QString& passwrod, const QString& title, const QString& details, const QString& version, const QString& os,
                            const QString& plugins, ResponseHandler responseHandler)
{

}

void BugReporter::requestFeature(const QString& email, const QString& title, const QString& details, ResponseHandler responseHandler)
{

}

void BugReporter::requestFeature(const QString& user, const QString& passwrod, const QString& title, const QString& details, ResponseHandler responseHandler)
{

}
