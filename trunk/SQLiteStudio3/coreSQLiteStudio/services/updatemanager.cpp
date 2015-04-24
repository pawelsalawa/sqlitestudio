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
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#ifdef Q_OS_WIN32
#include "JlCompress.h"
#include <windows.h>
#include <shellapi.h>
#endif

// Note on creating update packages:
// Packages for Linux and MacOSX should be an archive of _contents_ of SQLiteStudio directory,
// while for Windows it should be an archive of SQLiteStudio directory itself.

QString UpdateManager::staticErrorMessage;
UpdateManager::RetryFunction UpdateManager::retryFunction = nullptr;

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(this, SIGNAL(updatingError(QString)), NOTIFY_MANAGER, SLOT(error(QString)));
}

UpdateManager::~UpdateManager()
{
    cleanup();
}

void UpdateManager::checkForUpdates(bool force)
{
    getUpdatesMetadata(updatesCheckReply, force);
}

void UpdateManager::update()
{
    if (updatesGetUrlsReply || updatesInProgress)
        return;

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

#if defined(Q_OS_WIN32)
bool UpdateManager::executePreFinalStepWin(const QString &tempDir, const QString &backupDir, const QString &appDir, bool reqAdmin)
{
    bool res;
    if (reqAdmin)
        res = executeFinalStepAsRootWin(tempDir, backupDir, appDir);
    else
        res = executeFinalStep(tempDir, backupDir, appDir);

    if (res)
    {
        QFileInfo path(qApp->applicationFilePath());
        QProcess::startDetached(appDir + "/" + path.fileName(), {WIN_POST_FINAL_UPDATE_OPTION_NAME, tempDir});
    }
    return res;
}
#endif

void UpdateManager::handleAvailableUpdatesReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        updatingFailed(tr("An error occurred while checking for updates: %1.").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

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
    else
        emit noUpdatesAvailable();
}

void UpdateManager::getUpdatesMetadata(QNetworkReply*& replyStoragePointer, bool force)
{
#ifndef NO_AUTO_UPDATES
    if ((!CFG_CORE.General.CheckUpdatesOnStartup.get() && !force) || !isPlatformEligibleForUpdate() || replyStoragePointer)
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
    if (reply->error() != QNetworkReply::NoError)
    {
        updatingFailed(tr("An error occurred while reading updates metadata: %1.").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

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
        QtConcurrent::run(this, &UpdateManager::installUpdates);
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

    requireAdmin = doRequireAdminPrivileges();

    QTemporaryDir installTempDir;
    QString appDirName = QDir(getAppDirPath()).dirName();
    QString targetDir = installTempDir.path() + QLatin1Char('/') + appDirName;
    if (!copyRecursively(getAppDirPath(), targetDir))
    {
        updatingFailed(tr("Could not copy current application directory into %1 directory.").arg(installTempDir.path()));
        return;
    }
    emit updatingProgress(currentJobTitle, 40, totalPercent);

    int i = 0;
    int updatesCnt = updatesToInstall.size();
    for (const QString& component : updatesToInstall.keys())
    {
        if (!installComponent(component, targetDir))
        {
            cleanup();
            updatesInProgress = false;
            return;
        }
        i++;
        emit updatingProgress(currentJobTitle, (30 + (50 / updatesCnt * i)), totalPercent);
    }

    if (!executeFinalStep(targetDir))
    {
        cleanup();
        updatesInProgress = false;
        return;
    }

    currentJobTitle = QString();
    totalPercent = 100;
    emit updatingProgress(currentJobTitle, 100, totalPercent);
    cleanup();
    updatesInProgress = false;
#ifdef Q_OS_WIN32
    installTempDir.setAutoRemove(false);
#endif

    SQLITESTUDIO->setImmediateQuit(true);
    qApp->exit(0);
}

bool UpdateManager::executeFinalStep(const QString& tempDir, const QString& backupDir, const QString& appDir)
{
    bool isWin = false;
#ifdef Q_OS_WIN32
    isWin = true;

    // Windows needs to wait for previus process to exit
    QThread::sleep(3);

    QDir dir(backupDir);
    QString dirName = dir.dirName();
    dir.cdUp();
    if (!dir.mkdir(dirName))
    {
        staticUpdatingFailed(tr("Could not create directory %1.").arg(backupDir));
        return false;
    }
#endif
    while (!moveDir(appDir, backupDir, isWin))
    {
        if (!retryFunction)
        {
            staticUpdatingFailed(tr("Could not rename directory %1 to %2.\nDetails: %3").arg(appDir, backupDir, staticErrorMessage));
            return false;
        }

        if (!retryFunction(tr("Cannot not rename directory %1 to %2.\nDetails: %3").arg(appDir, backupDir, staticErrorMessage)))
            return false;
    }

    if (!moveDir(tempDir, appDir, isWin))
    {
        if (!moveDir(backupDir, appDir, isWin))
        {
            staticUpdatingFailed(tr("Could not move directory %1 to %2 and also failed to restore original directory, "
                              "so the original SQLiteStudio directory is now located at: %3").arg(tempDir, appDir, backupDir));
        }
        else
        {
            staticUpdatingFailed(tr("Could not rename directory %1 to %2. Rolled back to the original SQLiteStudio version.").arg(tempDir, appDir));
        }
        deleteDir(backupDir);
        return false;
    }

    deleteDir(backupDir);
    return true;
}

bool UpdateManager::handleUpdateOptions(const QStringList& argList, int& returnCode)
{
    if (argList.size() == 5 && argList[1] == UPDATE_OPTION_NAME)
    {
        bool result = UpdateManager::executeFinalStep(argList[2], argList[3], argList[4]);
        if (result)
            returnCode = 0;
        else
            returnCode = 1;

        return true;
    }

#ifdef Q_OS_WIN32
    if (argList.size() == 6 && argList[1] == WIN_PRE_FINAL_UPDATE_OPTION_NAME)
    {
        bool result = UpdateManager::executePreFinalStepWin(argList[2], argList[3], argList[4], (bool)argList[5].toInt());
        if (result)
            returnCode = 0;
        else
            returnCode = -1;

        return true;
    }

    if (argList.size() == 3 && argList[1] == WIN_POST_FINAL_UPDATE_OPTION_NAME)
    {
        QThread::sleep(1); // to make sure that the previous process has quit
        returnCode = 0;
        UpdateManager::executePostFinalStepWin(argList[2]);
        return true;
    }
#endif

    return false;
}

QString UpdateManager::getStaticErrorMessage()
{
    return staticErrorMessage;
}

bool UpdateManager::executeFinalStep(const QString& tempDir)
{
    QString appDir = getAppDirPath();

    // Find inexisting dir name next to app dir
    QDir backupDir(getBackupDir(appDir));

#if defined(Q_OS_WIN32)
    return runAnotherInstanceForUpdate(tempDir, backupDir.absolutePath(), qApp->applicationDirPath(), requireAdmin);
#else
    bool res;
    if (requireAdmin)
        res = executeFinalStepAsRoot(tempDir, backupDir.absolutePath(), appDir);
    else
        res = executeFinalStep(tempDir, backupDir.absolutePath(), appDir);

    if (res)
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());

    return res;
#endif
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
    requireAdmin = false;
}

bool UpdateManager::waitForProcess(QProcess& proc)
{
    if (!proc.waitForFinished(-1))
    {
        qDebug() << "Update QProcess timed out.";
        return false;
    }

    if (proc.exitStatus() == QProcess::CrashExit)
    {
        qDebug() << "Update QProcess finished by crashing.";
        return false;
    }

    if (proc.exitCode() != 0)
    {
        qDebug() << "Update QProcess finished with code:" << proc.exitCode();
        return false;
    }

    return true;
}

QString UpdateManager::readError(QProcess& proc, bool reverseOrder)
{
    QString err = QString::fromLocal8Bit(reverseOrder ? proc.readAllStandardOutput() : proc.readAllStandardError());
    if (err.isEmpty())
        err = QString::fromLocal8Bit(reverseOrder ? proc.readAllStandardError() : proc.readAllStandardOutput());

    QString errStr = proc.errorString();
    if (!errStr.isEmpty())
        err += "\n" + errStr;

    return err;
}

void UpdateManager::staticUpdatingFailed(const QString& errMsg)
{
#if defined(Q_OS_WIN32)
    staticErrorMessage = errMsg;
#else
    UPDATES->handleStaticFail(errMsg);
#endif
    qCritical() << errMsg;
}

bool UpdateManager::executeFinalStepAsRoot(const QString& tempDir, const QString& backupDir, const QString& appDir)
{
#if defined(Q_OS_LINUX)
    return executeFinalStepAsRootLinux(tempDir, backupDir, appDir);
#elif defined(Q_OS_WIN32)
    return executeFinalStepAsRootWin(tempDir, backupDir, appDir);
#elif defined(Q_OS_MACX)
    return executeFinalStepAsRootMac(tempDir, backupDir, appDir);
#else
    qCritical() << "Unknown update platform in UpdateManager::executeFinalStepAsRoot() for package" << packagePath;
    return false;
#endif
}

#if defined(Q_OS_LINUX)
bool UpdateManager::executeFinalStepAsRootLinux(const QString& tempDir, const QString& backupDir, const QString& appDir)
{
    QStringList args = {qApp->applicationFilePath(), UPDATE_OPTION_NAME, tempDir, backupDir, appDir};

    QProcess proc;
    LinuxPermElevator elevator = findPermElevatorForLinux();
    switch (elevator)
    {
        case LinuxPermElevator::KDESU:
            proc.setProgram("kdesu");
            args.prepend("-t");
            proc.setArguments(args);
            break;
        case LinuxPermElevator::GKSU:
            proc.setProgram("gksu"); // TODO test gksu updates
            proc.setArguments(args);
            break;
        case LinuxPermElevator::PKEXEC:
        {
            // We call CLI for doing final step, because pkexec runs cmd completly in root env, so there's no X server.
            args[0] += "cli";

            QStringList newArgs;
            for (const QString& arg : args)
                newArgs << wrapCmdLineArgument(arg);

            QString cmd = "cd " + wrapCmdLineArgument(qApp->applicationDirPath()) +"; " + newArgs.join(" ");

            proc.setProgram("pkexec");
            proc.setArguments({"sh", "-c", cmd});
        }
            break;
        case LinuxPermElevator::NONE:
            updatingFailed(tr("Could not find permissions elevator application to run update as a root. Looked for: %1").arg("kdesu, gksu, pkexec"));
            return false;
    }

    proc.start();
    if (!waitForProcess(proc))
    {
        updatingFailed(tr("Could not execute final updating steps as root: %1").arg(readError(proc, (elevator == LinuxPermElevator::KDESU))));
        return false;
    }

    return true;
}
#endif

#ifdef Q_OS_MACX
bool UpdateManager::executeFinalStepAsRootMac(const QString& tempDir, const QString& backupDir, const QString& appDir)
{
    // Prepare script for updater
    // osascript -e "do shell script \"stufftorunasroot\" with administrator privileges"
    QStringList args = {wrapCmdLineArgument(qApp->applicationFilePath() + "cli"),
                        UPDATE_OPTION_NAME,
                        wrapCmdLineArgument(tempDir),
                        wrapCmdLineArgument(backupDir),
                        wrapCmdLineArgument(appDir)};
    QProcess proc;

    QString innerCmd = wrapCmdLineArgument(args.join(" "));

    static_qstring(scriptTpl, "do shell script %1 with administrator privileges");
    QString scriptCmd = scriptTpl.arg(innerCmd);

    // Prepare updater temporary directory
    QTemporaryDir updaterDir;
    if (!updaterDir.isValid())
    {
        updatingFailed(tr("Could not execute final updating steps as admin: %1").arg(tr("Cannot create temporary directory for updater.")));
        return false;
    }

    // Create updater script
    QString scriptPath = updaterDir.path() + "/UpdateSQLiteStudio.scpt";
    QFile updaterScript(scriptPath);
    if (!updaterScript.open(QIODevice::WriteOnly))
    {
        updatingFailed(tr("Could not execute final updating steps as admin: %1").arg(tr("Cannot create updater script file.")));
        return false;
    }
    updaterScript.write(scriptCmd.toLocal8Bit());
    updaterScript.close();

    // Compile script to updater application
    QString updaterApp = updaterDir.path() + "/UpdateSQLiteStudio.app";
    proc.setProgram("osacompile");
    proc.setArguments({"-o", updaterApp, scriptPath});
    proc.start();
    if (!waitForProcess(proc))
    {
        updatingFailed(tr("Could not execute final updating steps as admin: %1").arg(readError(proc)));
        return false;
    }

    // Execute updater
    proc.setProgram(updaterApp + "/Contents/MacOS/applet");
    proc.setArguments({});
    proc.start();
    if (!waitForProcess(proc))
    {
        updatingFailed(tr("Could not execute final updating steps as admin: %1").arg(readError(proc)));
        return false;
    }

    // Validating update
    // The updater script will not return error if the user canceled the password prompt.
    // We need to check if the update was actually made and return true only then.
    if (QDir(tempDir).exists())
    {
        // Temp dir still exists, so it was not moved by root process
        updatingFailed(tr("Updating canceled."));
        return false;
    }

    return true;
}
#endif

#ifdef Q_OS_WIN32
bool UpdateManager::executeFinalStepAsRootWin(const QString& tempDir, const QString& backupDir, const QString& appDir)
{
    QString updateBin = qApp->applicationDirPath() + "/" + WIN_UPDATER_BINARY;

    QString installFilePath = tempDir + "/" + WIN_INSTALL_FILE;
    QFile installFile(installFilePath);
    installFile.open(QIODevice::WriteOnly);
    QString nl("\n");
    installFile.write(UPDATE_OPTION_NAME);
    installFile.write(nl.toLocal8Bit());
    installFile.write(backupDir.toLocal8Bit());
    installFile.write(nl.toLocal8Bit());
    installFile.write(appDir.toLocal8Bit());
    installFile.write(nl.toLocal8Bit());
    installFile.close();

    int res = (int)::ShellExecuteA(0, "runas", updateBin.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
    if (res < 32)
    {
        staticUpdatingFailed(tr("Could not execute final updating steps as administrator."));
        return false;
    }

    // Since I suck as a developer and I cannot implement a simple synchronous app call under Windows
    // (QProcess does it somehow, but I'm too lazy to look it up and probably the solution wouldn't be compatible
    // with our "privileges elevation" trick above... so after all I think we're stuck with this solution for now),
    // I do the workaround here, which makes this process wait for the other process to create the "done"
    // file when it's done, so this process knows when the other has ended. This way we can proceed with this
    // process and we will delete some directories later on, which were required by that other process.
    if (!waitForFileToDisappear(installFilePath, 10))
    {
        staticUpdatingFailed(tr("Could not execute final updating steps as administrator. Updater startup timed out."));
        return false;
    }

    if (!waitForFileToAppear(appDir + QLatin1Char('/') + WIN_UPDATE_DONE_FILE, 30))
    {
        staticUpdatingFailed(tr("Could not execute final updating steps as administrator. Updater operation timed out."));
        return false;
    }

    return true;
}
#endif

#if defined(Q_OS_WIN32)
bool UpdateManager::executePostFinalStepWin(const QString &tempDir)
{
    QString doneFile = qApp->applicationDirPath() + QLatin1Char('/') + WIN_UPDATE_DONE_FILE;
    QFile::remove(doneFile);

    QDir dir(tempDir);
    dir.cdUp();
    if (!deleteDir(dir.absolutePath()))
        staticUpdatingFailed(tr("Could not clean up temporary directory %1. You can delete it manually at any time.").arg(dir.absolutePath()));

    QProcess::startDetached(qApp->applicationFilePath(), QStringList());
    return true;
}

bool UpdateManager::waitForFileToDisappear(const QString &filePath, int seconds)
{
    QFile file(filePath);
    while (file.exists() && seconds > 0)
    {
        QThread::sleep(1);
        seconds--;
    }

    return !file.exists();
}

bool UpdateManager::waitForFileToAppear(const QString &filePath, int seconds)
{
    QFile file(filePath);
    while (!file.exists() && seconds > 0)
    {
        QThread::sleep(1);
        seconds--;
    }

    return file.exists();
}

bool UpdateManager::runAnotherInstanceForUpdate(const QString &tempDir, const QString &backupDir, const QString &appDir, bool reqAdmin)
{
    bool res = QProcess::startDetached(tempDir + "/SQLiteStudio.exe", {WIN_PRE_FINAL_UPDATE_OPTION_NAME, tempDir, backupDir, appDir,
                                                                       QString::number((int)reqAdmin)});
    if (!res)
    {
        updatingFailed(tr("Could not run new version for continuing update."));
        return false;
    }

    return true;
}
#endif

UpdateManager::LinuxPermElevator UpdateManager::findPermElevatorForLinux()
{
#if defined(Q_OS_LINUX)
    QProcess proc;
    proc.setProgram("which");

    if (!SQLITESTUDIO->getEnv("DISPLAY").isEmpty())
    {
        proc.setArguments({"kdesu"});
        proc.start();
        if (waitForProcess(proc))
            return LinuxPermElevator::KDESU;

        proc.setArguments({"gksu"});
        proc.start();
        if (waitForProcess(proc))
            return LinuxPermElevator::GKSU;
    }

    proc.setArguments({"pkexec"});
    proc.start();
    if (waitForProcess(proc))
        return LinuxPermElevator::PKEXEC;
#endif

    return LinuxPermElevator::NONE;
}

QString UpdateManager::wrapCmdLineArgument(const QString& arg)
{
    return "\"" + escapeCmdLineArgument(arg) + "\"";
}

QString UpdateManager::escapeCmdLineArgument(const QString& arg)
{
    if (!arg.contains("\\") && !arg.contains("\""))
        return arg;

    QString str = arg;
    return str.replace("\\", "\\\\").replace("\"", "\\\"");
}

QString UpdateManager::getBackupDir(const QString &appDir)
{
    static_qstring(bakDirTpl, "%1.old%2");
    QDir backupDir(bakDirTpl.arg(appDir, ""));
    int cnt = 1;
    while (backupDir.exists())
        backupDir = QDir(bakDirTpl.arg(appDir, QString::number(cnt)));

    return backupDir.absolutePath();
}

bool UpdateManager::unpackToDir(const QString& packagePath, const QString& outputDir)
{
#if defined(Q_OS_LINUX)
    return unpackToDirLinux(packagePath, outputDir);
#elif defined(Q_OS_WIN32)
    return unpackToDirWin(packagePath, outputDir);
#elif defined(Q_OS_MACX)
    return unpackToDirMac(packagePath, outputDir);
#else
    qCritical() << "Unknown update platform in UpdateManager::unpackToDir() for package" << packagePath;
    return false;
#endif
}

#if defined(Q_OS_LINUX)
bool UpdateManager::unpackToDirLinux(const QString &packagePath, const QString &outputDir)
{
    QProcess proc;
    proc.setWorkingDirectory(outputDir);
    proc.setStandardOutputFile(QProcess::nullDevice());
    proc.setStandardErrorFile(QProcess::nullDevice());

    if (!packagePath.endsWith("tar.gz"))
    {
        updatingFailed(tr("Package not in tar.gz format, cannot install: %1").arg(packagePath));
        return false;
    }

    proc.start("mv", {packagePath, outputDir});
    if (!waitForProcess(proc))
    {
        updatingFailed(tr("Package %1 cannot be installed, because cannot move it to directory: %2").arg(packagePath, outputDir));
        return false;
    }

    QString fileName = packagePath.split("/").last();
    QString newPath = outputDir + "/" + fileName;
    proc.start("tar", {"-xzf", newPath});
    if (!waitForProcess(proc))
    {
        updatingFailed(tr("Package %1 cannot be installed, because cannot unpack it: %2").arg(packagePath, readError(proc)));
        return false;
    }

    QProcess::execute("rm", {"-f", newPath});
    return true;
}
#endif

#if defined(Q_OS_MACX)
bool UpdateManager::unpackToDirMac(const QString &packagePath, const QString &outputDir)
{
    QProcess proc;
    proc.setWorkingDirectory(outputDir);
    proc.setStandardOutputFile(QProcess::nullDevice());
    proc.setStandardErrorFile(QProcess::nullDevice());

    if (!packagePath.endsWith("zip"))
    {
        updatingFailed(tr("Package not in zip format, cannot install: %1").arg(packagePath));
        return false;
    }

    proc.start("unzip", {"-o", "-d", outputDir, packagePath});
    if (!waitForProcess(proc))
    {
        updatingFailed(tr("Package %1 cannot be installed, because cannot unzip it to directory %2: %3")
                       .arg(packagePath, outputDir, readError(proc)));
        return false;
    }

    return true;
}
#endif

#if defined(Q_OS_WIN32)
bool UpdateManager::unpackToDirWin(const QString& packagePath, const QString& outputDir)
{
    if (JlCompress::extractDir(packagePath, outputDir + "/..").isEmpty())
    {
        updatingFailed(tr("Package %1 cannot be installed, because cannot unzip it to directory: %2").arg(packagePath, outputDir));
        return false;
    }

    return true;
}
#endif

void UpdateManager::handleStaticFail(const QString& errMsg)
{
    emit updatingFailed(errMsg);
}

QString UpdateManager::getAppDirPath() const
{
    static QString appDir;
    if (appDir.isNull())
    {
        appDir = qApp->applicationDirPath();
#ifdef Q_OS_MACX
        QDir tmpAppDir(appDir);
        tmpAppDir.cdUp();
        tmpAppDir.cdUp();
        appDir = tmpAppDir.absolutePath();
#endif
    }
    return appDir;
}

bool UpdateManager::moveDir(const QString& src, const QString& dst, bool contentsOnly)
{
    // If we're doing a rename in the very same parent directory then we don't want
    // the 'move between partitions' to be involved, cause any failure to rename
    // is due to permissions or file lock.
    QFileInfo srcFi(src);
    QFileInfo dstFi(dst);
    bool sameParentDir = (srcFi.dir() == dstFi.dir());

    QDir dir;
    if (contentsOnly)
    {
        QString localSrc;
        QString localDst;
        QDir srcDir(src);
        for (const QFileInfo& entry : srcDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System))
        {
            localSrc = entry.absoluteFilePath();
            localDst = dst + "/" + entry.fileName();
            if (!dir.rename(localSrc, localDst) && (sameParentDir || !renameBetweenPartitions(localSrc, localDst)))
            {
                staticUpdatingFailed(tr("Could not rename directory %1 to %2.").arg(localSrc, localDst));
                return false;
            }
        }
    }
    else
    {
        if (!dir.rename(src, dst) && (sameParentDir || !renameBetweenPartitions(src, dst)))
        {
            staticUpdatingFailed(tr("Could not rename directory %1 to %2.").arg(src, dst));
            return false;
        }
    }

    return true;
}

bool UpdateManager::deleteDir(const QString& path)
{
    QDir dir(path);
    if (!dir.removeRecursively())
    {
        staticUpdatingFailed(tr("Could not delete directory %1.").arg(path));
        return false;
    }

    return true;
}

bool UpdateManager::execCmd(const QString& cmd, const QStringList& args, QString* errorMsg)
{
    QProcess proc;
    proc.start(cmd, args);
    QString cmdString = QString("%1 \"%2\"").arg(cmd, args.join("\\\" \\\""));

    if (!waitForProcess(proc))
    {
        if (errorMsg)
            *errorMsg = tr("Error executing update command: %1\nError message: %2").arg(cmdString).arg(readError(proc));

        return false;
    }

    return true;
}

void UpdateManager::setRetryFunction(const RetryFunction &value)
{
    retryFunction = value;
}

bool UpdateManager::doRequireAdminPrivileges()
{
    QString appDirPath = getAppDirPath();
    QDir appDir(appDirPath);
    bool isWritable = isWritableRecursively(appDir.absolutePath());

    appDir.cdUp();
    QFileInfo fi(appDir.absolutePath());
    isWritable &= fi.isWritable();

    if (isWritable)
    {
        QDir backupDir(getBackupDir(appDirPath));
        QString backupDirName = backupDir.dirName();
        backupDir.cdUp();
        if (backupDir.mkdir(backupDirName))
            backupDir.rmdir(backupDirName);
        else
            isWritable = false;
    }

    return !isWritable;
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
