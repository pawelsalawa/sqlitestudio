#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateManager : public QObject
{
        Q_OBJECT
    public:
        struct UpdateEntry
        {
            QString compontent;
            QString version;
        };

        typedef std::function<QString()> AdminPassHandler;

        explicit UpdateManager(QObject *parent = 0);

        void checkForUpdates();
        void update(AdminPassHandler adminPassHandler);

    private:
        QString getPlatformForUpdate() const;
        QString getCurrentVersions() const;
        bool isPlatformEligibleForUpdate() const;
        void handleAvailableUpdatesReply(QNetworkReply* reply);

        QNetworkAccessManager* networkManager = nullptr;
        QNetworkReply* updatesCheckReply = nullptr;
        QNetworkReply* updatesGetReply = nullptr;

        static_char* updateServiceUrl = "http://sqlitestudio.pl/updates3.rvt";

    private slots:
        void finished(QNetworkReply* reply);

    signals:
        void updatesAvailable(const QList<UpdateManager::UpdateEntry>& updates);
};

#define UPDATES SQLITESTUDIO->getUpdateManager()

#endif // UPDATEMANAGER_H
