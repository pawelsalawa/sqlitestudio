#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#ifdef PORTABLE_CONFIG

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <functional>
#include <QProcess>

class QNetworkAccessManager;
class QNetworkReply;
class QTemporaryDir;
class QFile;

class API_EXPORT UpdateManager : public QObject
{
        Q_OBJECT
    public:
        struct UpdateEntry
        {
            QString compontent;
            QString version;
        };

        explicit UpdateManager(QObject *parent = 0);
        ~UpdateManager();

        void checkForUpdates();
        void update();
        bool isPlatformEligibleForUpdate() const;

    private:
        QString updateBinaryAbsolutePath;

        bool waitForProcess(QProcess& proc);
        void processCheckResults(const QByteArray& results);

    signals:
        void updatesAvailable(const QList<UpdateManager::UpdateEntry>& updates);
        void noUpdatesAvailable();
        void updatingError(const QString& errorMessage);
};

#define UPDATES SQLITESTUDIO->getUpdateManager()

#endif // PORTABLE_CONFIG
#endif // UPDATEMANAGER_H
