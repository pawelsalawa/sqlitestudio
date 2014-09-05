#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;

class API_EXPORT UpdateManager : public QObject
{
        Q_OBJECT
    public:
        struct UpdateEntry
        {
            QString compontent;
            QString version;
            QString url;
        };

        typedef std::function<QString()> AdminPassHandler;

        explicit UpdateManager(QObject *parent = 0);

        void checkForUpdates();
        void update(AdminPassHandler adminPassHandler);
        bool isPlatformEligibleForUpdate() const;

    private:
        QString getPlatformForUpdate() const;
        QString getCurrentVersions() const;
        void handleAvailableUpdatesReply(QNetworkReply* reply);
        void getUpdatesMetadata(QNetworkReply*& replyStoragePointer);
        void handleUpdatesMetadata(QNetworkReply* reply);
        QList<UpdateEntry> readMetadata(const QJsonDocument& doc);
        void downloadUpdates();

        QNetworkAccessManager* networkManager = nullptr;
        QNetworkReply* updatesCheckReply = nullptr;
        QNetworkReply* updatesGetUrlsReply = nullptr;
        QNetworkReply* updatesGetReply = nullptr;
        UpdateManager::AdminPassHandler adminPassHandler;
        bool updatesInProgress = false;
        QList<UpdateEntry> updatesToDownload;

        static_char* updateServiceUrl = "http://sqlitestudio.pl/updates3.rvt";
        static_char* manualUpdatesHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Manual";

    private slots:
        void finished(QNetworkReply* reply);

    signals:
        void updatesAvailable(const QList<UpdateManager::UpdateEntry>& updates);
};

#define UPDATES SQLITESTUDIO->getUpdateManager()

#endif // UPDATEMANAGER_H
