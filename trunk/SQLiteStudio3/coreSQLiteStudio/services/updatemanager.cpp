#include "updatemanager.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

void UpdateManager::checkForUpdates()
{
    getUpdatesMetadata(updatesCheckReply);
}

void UpdateManager::update(UpdateManager::AdminPassHandler adminPassHandler)
{
    if (updatesGetUrlsReply || updatesInProgress)
        return;

    this->adminPassHandler = adminPassHandler;
    getUpdatesMetadata(updatesGetUrlsReply);
}

QString UpdateManager::getPlatformForUpdate() const
{
#if defined(Q_OS_LINUX)
    if (QSysInfo::WordSize == 64)
        return "linux64";
    else
        return "linux32";
#elif defined(Q_OS_WIN)
    return "win32";
#elif defined(Q_OS_OSX)
    return "macosx";
#else
    return QString();
#endif
}

QString UpdateManager::getCurrentVersions() const
{
    QJsonArray versionsArray;

    QJsonObject arrayEntry;
    arrayEntry["component"] = "SQLiteStudio";
    arrayEntry["version"] = SQLITESTUDIO->getVersionString();
    versionsArray.append(arrayEntry);

    for (const PluginManager::PluginDetails& details : PLUGINS->getAllPluginDetails())
    {
        if (details.builtIn)
            continue;

        arrayEntry["component"] = details.name;
        arrayEntry["version"] = details.versionString;
        versionsArray.append(arrayEntry);
    }

    QJsonObject topObj;
    topObj["versions"] = versionsArray;

    QJsonDocument doc(topObj);
    return QString::fromLatin1(doc.toJson(QJsonDocument::Compact));
}

bool UpdateManager::isPlatformEligibleForUpdate() const
{
    return !getPlatformForUpdate().isNull() && getDistributionType() != DistributionType::OS_MANAGED;
}

void UpdateManager::handleAvailableUpdatesReply(QNetworkReply* reply)
{
    QJsonParseError err;
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
    {
        qWarning() << "Invalid response from update service:" << err.errorString() << "\n" << "The data was:" << QString::fromLatin1(data);
        notifyWarn(tr("Could not check available updates, because server responded with invalid message format. It is safe to ignore this warning."));
        return;
    }

    QList<UpdateEntry> updates = readMetadata(doc);
    emit updatesAvailable(updates);
}

void UpdateManager::getUpdatesMetadata(QNetworkReply*& replyStoragePointer)
{
#ifndef NO_AUTO_UPDATES
    if (!isPlatformEligibleForUpdate() || replyStoragePointer)
        return;

    QUrlQuery query;
    query.addQueryItem("platform", getPlatformForUpdate());
    query.addQueryItem("data", getCurrentVersions());

    QUrl url(QString::fromLatin1(updateServiceUrl) + "?" + query.query(QUrl::FullyEncoded));
    QNetworkRequest request(url);
    replyStoragePointer = networkManager->get(request);
#endif
}

void UpdateManager::handleUpdatesMetadata(QNetworkReply* reply)
{
    QJsonParseError err;
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
    {
        qWarning() << "Invalid response from update service for getting metadata:" << err.errorString() << "\n" << "The data was:" << QString::fromLatin1(data);
        notifyWarn(tr("Could not download updates, because server responded with invalid message format. "
                      "You can try again later or download and install updates manually. See <a href=\"%1\">User Manual</a> for details.").arg(manualUpdatesHelpUrl));
        return;
    }

    updatesInProgress = true;
    updatesToDownload = readMetadata(doc);
    downloadUpdates();
}

QList<UpdateManager::UpdateEntry> UpdateManager::readMetadata(const QJsonDocument& doc)
{
    QList<UpdateEntry> updates;
    UpdateEntry entry;
    QJsonObject obj = doc.object();
    QJsonArray versionsArray = obj["newVersions"].toArray();
    QJsonObject entryObj;
    for (const QJsonValue& value : versionsArray)
    {
        entryObj = value.toObject();
        entry.compontent = entryObj["component"].toString();
        entry.version = entryObj["version"].toString();
        entry.url = entryObj["url"].toString();
        updates << entry;
    }

    return updates;
}

void UpdateManager::downloadUpdates()
{

}

void UpdateManager::finished(QNetworkReply* reply)
{
    if (reply == updatesCheckReply)
    {
        updatesCheckReply = nullptr;
        handleAvailableUpdatesReply(reply);
        return;
    }

    if (reply == updatesGetUrlsReply)
    {
        updatesGetUrlsReply = nullptr;
        handleUpdatesMetadata(reply);
        return;
    }
}
