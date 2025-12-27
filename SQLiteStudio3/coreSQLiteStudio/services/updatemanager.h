#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#ifdef PORTABLE_CONFIG

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <functional>
#include <QProcess>

class QNetworkReply;
class QNetworkAccessManager;
class QTemporaryDir;
class QFile;

class API_EXPORT UpdateManager : public QObject
{
        Q_OBJECT
    public:
        explicit UpdateManager(QObject *parent = 0);
        ~UpdateManager();

        void checkForUpdates(bool enforce = false);
        bool isPlatformEligibleForUpdate() const;

    private:
        QString updateBinaryAbsolutePath;
        QNetworkAccessManager *netManager = nullptr;

        void handleUpdatesResponse(QNetworkReply* response, bool enforced);

    private slots:
        void handleUpdatingError(const QString& errorMessage);

    signals:
        void updateAvailable(const QString& version, const QString& url);
        void noUpdatesAvailable(bool enforced);
        void updatingError(const QString& errorMessage);
};

#define UPDATES SQLITESTUDIO->getUpdateManager()

#endif // PORTABLE_CONFIG
#endif // UPDATEMANAGER_H
