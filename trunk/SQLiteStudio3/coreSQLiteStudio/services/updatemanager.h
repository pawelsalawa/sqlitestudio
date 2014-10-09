#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <functional>

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
            QString url;
        };

        typedef std::function<QString()> AdminPassHandler;

        explicit UpdateManager(QObject *parent = 0);
        ~UpdateManager();

        void checkForUpdates();
        void update(AdminPassHandler adminPassHandler);
        bool isPlatformEligibleForUpdate() const;

    private:
        QString getPlatformForUpdate() const;
        QString getCurrentVersions() const;
        void handleAvailableUpdatesReply(QNetworkReply* reply);
        void handleDownloadReply(QNetworkReply* reply);
        void getUpdatesMetadata(QNetworkReply*& replyStoragePointer);
        void handleUpdatesMetadata(QNetworkReply* reply);
        QList<UpdateEntry> readMetadata(const QJsonDocument& doc);
        void downloadUpdates();
        void updatingFailed(const QString& errMsg);
        void installUpdates();
        bool installComponent(const QString& component, const QString& tempDir);
        bool moveTempInstallationToFinal(const QString& tempDir);
        bool doRequireAdminPrivileges();
        bool unpackToDir(const QString& packagePath, const QString& outputDir);
        bool moveDir(const QString& src, const QString& dst);
        bool moveDirLinux(const QString& src, const QString& dst);
        bool deleteDir(const QString& path);
        bool deleteDirLinux(const QString& path);
        bool execLinux(const QString& cmd, const QStringList& args, QString* errorMsg = nullptr);
        void cleanup();

        QNetworkAccessManager* networkManager = nullptr;
        QNetworkReply* updatesCheckReply = nullptr;
        QNetworkReply* updatesGetUrlsReply = nullptr;
        QNetworkReply* updatesGetReply = nullptr;
        UpdateManager::AdminPassHandler adminPassHandler;
        bool updatesInProgress = false;
        QList<UpdateEntry> updatesToDownload;
        QHash<QString,QString> updatesToInstall;
        QHash<QString,QString> componentToPath;
        QTemporaryDir* tempDir = nullptr;
        QFile* currentDownloadFile = nullptr;
        int totalPercent = 0;
        int totalDownloadsCount = 0;
        QString currentJobTitle;
        QString adminPassword;
        bool requireAdmin = false;

        static_char* updateServiceUrl = "http://sqlitestudio.pl/updates3.rvt";
        static_char* manualUpdatesHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Manual";

    private slots:
        void finished(QNetworkReply* reply);
        void downloadProgress(qint64 bytesReceived, qint64 totalBytes);
        void readDownload();

    signals:
        void updatesAvailable(const QList<UpdateManager::UpdateEntry>& updates);
        void updatingProgress(const QString& jobTitle, int jobPercent, int totalPercent);
        void updatingFinished();
        void updatingError(const QString& errorMessage);
};

#define UPDATES SQLITESTUDIO->getUpdateManager()

#endif // UPDATEMANAGER_H
