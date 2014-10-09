#include "updatemanager.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include <QTemporaryDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QProcess>

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

UpdateManager::~UpdateManager()
{
    cleanup();
}

void UpdateManager::checkForUpdates()
{
    getUpdatesMetadata(updatesCheckReply);
}

void UpdateManager::update(UpdateManager::AdminPassHandler adminPassHandler)
{
    if (updatesGetUrlsReply || updatesInProgress)
        return;

    if (!adminPassHandler)
    {
        qCritical() << "Null admin password handler. Updating aborted.";
        return;
    }

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
    if (updates.size() > 0)
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

    tempDir = new QTemporaryDir();
    if (!tempDir->isValid()) {
        notifyWarn(tr("Could not create temporary directory for downloading the update. Updating aborted."));
        return;
    }

    updatesInProgress = true;
    updatesToDownload = readMetadata(doc);
    totalDownloadsCount = updatesToDownload.size();
    totalPercent = 0;

    if (totalDownloadsCount == 0)
    {
        updatingFailed(tr("There was no updates to download. Updating aborted."));
        return;
    }

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
    if (updatesToDownload.size() == 0)
    {
        installUpdates();
        return;
    }

    UpdateEntry entry = updatesToDownload.takeFirst();
    currentJobTitle = tr("Downloading: %1").arg(entry.compontent);
    emit updatingProgress(currentJobTitle, 0, totalPercent);

    QStringList parts = entry.url.split("/");
    if (parts.size() < 1)
    {
        updatingFailed(tr("Could not determinate file name from update URL: %1. Updating aborted.").arg(entry.url));
        return;
    }

    QString path = tempDir->path() + QLatin1Char('/') + parts.last();
    currentDownloadFile = new QFile(path);
    if (!currentDownloadFile->open(QIODevice::WriteOnly))
    {
        updatingFailed(tr("Failed to open file '%1' for writting: %2. Updating aborted.").arg(path, currentDownloadFile->errorString()));
        return;
    }

    updatesToInstall[entry.compontent] = path;

    QNetworkRequest request(QUrl(entry.url));
    updatesGetReply = networkManager->get(request);
    connect(updatesGetReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(updatesGetReply, SIGNAL(readyRead()), this, SLOT(readDownload()));
}

void UpdateManager::updatingFailed(const QString& errMsg)
{
    cleanup();
    updatesInProgress = false;
    emit updatingError(errMsg);
}

void UpdateManager::installUpdates()
{
    currentJobTitle = tr("Installing updates.");
    totalPercent = (totalDownloadsCount - updatesToDownload.size()) * 100 / (totalDownloadsCount + 1);
    emit updatingProgress(currentJobTitle, 0, totalPercent);

//    collectComponentPaths();
    requireAdmin = doRequireAdminPrivileges();
    if (requireAdmin && adminPassword.isNull())
    {
        adminPassword = adminPassHandler();
        if (adminPassword.isNull())
        {
            updatingFailed(tr("User aborted updates installation."));
            return;
        }
    }

    QTemporaryDir installTempDir;
    QString appDirName = QDir(qApp->applicationDirPath()).dirName();
    QString targetDir = installTempDir.path() + QLatin1Char('/') + appDirName;
    if (!copyRecursively(qApp->applicationDirPath(), targetDir))
    {
        updatingFailed(tr("Could not copy current application directory into %1 directory.").arg(installTempDir.path()));
        return;
    }

    for (const QString& component : updatesToInstall.keys())
    {
        if (!installComponent(component, targetDir))
            return;
    }

    if (!moveTempInstallationToFinal(targetDir))
        return;

    currentJobTitle = QString();
    totalPercent = 100;
    emit updatingProgress(currentJobTitle, 100, totalPercent);
    cleanup();
    updatesInProgress = false;
}

bool UpdateManager::moveTempInstallationToFinal(const QString& tempDir)
{
    // Find inexisting dir name next to app dir
    static_qstring(bakDirTpl, "%1.old%2");
    QDir backupDir(bakDirTpl.arg(qApp->applicationDirPath(), ""));
    int cnt = 1;
    while (backupDir.exists())
        backupDir = QDir(bakDirTpl.arg(qApp->applicationDirPath(), QString::number(cnt)));

    if (!moveDir(qApp->applicationDirPath(), backupDir.absolutePath()))
    {
        updatingFailed(tr("Could not rename directory %1 to %2.").arg(qApp->applicationDirPath(), backupDir.dirName()));
        return false;
    }

    if (!moveDir(tempDir, qApp->applicationDirPath()))
    {
        if (!moveDir(backupDir.absolutePath(), qApp->applicationDirPath()))
        {
            updatingFailed(tr("Could not move directory %1 to %2 and also failed to restore original directory, "
                              "so the original SQLiteStudio directory is now located at: %3").arg(tempDir, qApp->applicationDirPath(), backupDir.absolutePath()));
        }
        else
        {
            updatingFailed(tr("Could not rename directory %1 to %2. Rolled back to the original SQLiteStudio version.").arg(tempDir, qApp->applicationDirPath()));
        }
        return false;
    }

    deleteDir(backupDir.absolutePath());

    return true;
}

bool UpdateManager::installComponent(const QString& component, const QString& tempDir)
{
    if (!unpackToDir(updatesToInstall[component], tempDir))
    {
        updatingFailed(tr("Could not unpack component %1 into %2 directory.").arg(component, tempDir));
        return false;
    }

    // In future here we might also delete/change some files, according to some update script.
    return true;
}

void UpdateManager::cleanup()
{
    safe_delete(currentDownloadFile);
    safe_delete(tempDir);
    updatesToDownload.clear();
    updatesToInstall.clear();
    componentToPath.clear();
    adminPassword = QString();
    adminPassHandler = nullptr;
    requireAdmin = false;
}

bool UpdateManager::unpackToDir(const QString& packagePath, const QString& outputDir)
{
    QProcess proc;
    proc.setWorkingDirectory(outputDir);
    proc.setStandardOutputFile(QProcess::nullDevice());
    proc.setStandardErrorFile(QProcess::nullDevice());

#if defined(Q_OS_LINUX)
    if (!packagePath.endsWith("tar.gz"))
    {
        updatingFailed(tr("Package not in tar.gz format, cannot install: %1").arg(packagePath));
        return false;
    }

    proc.start("mv", {packagePath, outputDir});
    if (!proc.waitForFinished(-1))
    {
        updatingFailed(tr("Package %1 cannot be installed, because cannot move it to directory: %2").arg(packagePath, outputDir));
        return false;
    }

    QString fileName = packagePath.split("/").last();
    QString newPath = outputDir + "/" + fileName;
    proc.start("tar", {"-xzf", newPath});
    if (!proc.waitForFinished(-1))
    {
        updatingFailed(tr("Package %1 cannot be installed, because cannot unpack it: %2").arg(packagePath, proc.errorString()));
        return false;
    }

    QProcess::execute("rm", {"-f", newPath});
#elif defined(Q_OS_WIN32)
    // TODO implement for win32
#elif defined(Q_OS_MACX)
    // TODO implement for macx
#else
    qCritical() << "Unknown update platform in UpdateManager::unpackToDir() for package" << packagePath;
    return false;
#endif
    return true;
}

bool UpdateManager::moveDir(const QString& src, const QString& dst)
{
#if defined(Q_OS_LINUX)
    return moveDirLinux(src, dst);
#elif defined(Q_OS_WIN32)
    // TODO implement for win32
#elif defined(Q_OS_MACX)
    // TODO implement for macx
#else
    qCritical() << "Unknown update platform in UpdateManager::installApplicationComponent()";
    return false;
#endif
}

bool UpdateManager::moveDirLinux(const QString& src, const QString& dst)
{
    QString msg;
    if (!execLinux("mv", {src, dst}, &msg))
    {
        updatingFailed(msg);
        return false;
    }

    return true;
}

bool UpdateManager::deleteDir(const QString& path)
{
#if defined(Q_OS_LINUX)
    return deleteDirLinux(path);
#elif defined(Q_OS_WIN32)
    // TODO implement for win32
#elif defined(Q_OS_MACX)
    // TODO implement for macx
#else
    qCritical() << "Unknown update platform in UpdateManager::installApplicationComponent()";
    return false;
#endif
}

bool UpdateManager::deleteDirLinux(const QString& path)
{
    QString msg;
    if (!execLinux("rm", {"-rf", path}, &msg))
    {
        qWarning() << "Problem with deleting dir in update:" << msg;
        return false;
    }

    return true;
}

bool UpdateManager::execLinux(const QString& cmd, const QStringList& args, QString* errorMsg)
{
    QProcess proc;
    QString cmdString;
    if (requireAdmin)
    {
        QProcess passProc;
        passProc.setStandardOutputProcess(&proc);
        passProc.start("echo", {adminPassword});

        proc.setProcessChannelMode(QProcess::ForwardedChannels);

        QStringList newArgs;
        newArgs << "-c";

        QStringList innerArgs;
        innerArgs << cmd;
        QString newPath;
        for (const QString& path : args)
        {
            newPath = path;
            innerArgs << "\"" + newPath.replace('"', "\\\"") + "\"";
        }

        newArgs << innerArgs.join(" ");
        proc.start("su", newArgs);

        passProc.waitForFinished(-1);

        cmdString = QString("su -c \"%1 \\\"%2\\\"\"").arg(cmd, args.join("\\\" \\\""));
    }
    else
    {
        proc.start(cmd, args);
        cmdString = QString("%1 \"%2\"").arg(cmd, args.join("\\\" \\\""));
    }

    if (!proc.waitForFinished(-1))
    {
        if (errorMsg)
            *errorMsg = tr("Error executing update command: %1\nError message: %2").arg(cmdString).arg(proc.errorString());

        return false;
    }

    return true;
}

bool UpdateManager::doRequireAdminPrivileges()
{
    QFileInfo fi;
    for (const QString& path : componentToPath.values())
    {
        fi.setFile(path);
        if (!fi.exists())
            fi.setFile(fi.dir().path());

        if (!fi.isWritable())
            return true;
    }

    return false;
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

    if (reply == updatesGetReply)
    {
        handleDownloadReply(reply);
        if (reply == updatesGetReply) // if no new download is requested
            updatesGetReply = nullptr;

        return;
    }
}

void UpdateManager::handleDownloadReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        updatingFailed(tr("An error occurred while downloading updates: %1. Updating aborted.").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    totalPercent = (totalDownloadsCount - updatesToDownload.size()) * 100 / (totalDownloadsCount + 1);

    readDownload();
    currentDownloadFile->close();

    safe_delete(currentDownloadFile);

    reply->deleteLater();
    downloadUpdates();
}

void UpdateManager::downloadProgress(qint64 bytesReceived, qint64 totalBytes)
{
    int perc;
    if (totalBytes < 0)
        perc = -1;
    else if (totalBytes == 0)
        perc = 100;
    else
        perc = bytesReceived * 100 / totalBytes;

    emit updatingProgress(currentJobTitle, perc, totalPercent);
}

void UpdateManager::readDownload()
{
    currentDownloadFile->write(updatesGetReply->readAll());
}
