#ifdef PORTABLE_CONFIG

#include "updatemanager.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include <QDebug>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrentRun>

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
    connect(this, SIGNAL(updatingError(QString)), NOTIFY_MANAGER, SLOT(error(QString)));

    QString updateBinary =
#if defined(Q_OS_WIN)
        "UpdateSQLiteStudio.exe";
#elif defined(Q_OS_LINUX)
        "UpdateSQLiteStudio";
#elif defined(Q_OS_OSX)
        "../../UpdateSQLiteStudio.app/Contents/MacOS/UpdateSQLiteStudio";
#else
        "";
#endif

    if (!updateBinary.isEmpty()) {
        updateBinaryAbsolutePath = QFileInfo(QCoreApplication::applicationDirPath() + "/" + updateBinary).absoluteFilePath();
    }
}

UpdateManager::~UpdateManager()
{

}

void UpdateManager::checkForUpdates()
{
    if (!CFG_CORE.General.CheckUpdatesOnStartup.get())
        return;

    if (updateBinaryAbsolutePath.isEmpty()) {
        qDebug() << "Updater binary not defined. Skipping updates checking.";
        return;
    }

    if (!QFileInfo(updateBinaryAbsolutePath).exists()) {
        QString errorDetails = tr("Updates installer executable is missing.");
        emit updatingError(tr("Unable to check for updates (%1)").arg(errorDetails.trimmed()));
        qWarning() << "Error while checking for updates: " << errorDetails;
        return;
    }

    QtConcurrent::run(this, &UpdateManager::checkForUpdatesAsync);
}

void UpdateManager::checkForUpdatesAsync()
{
    QProcess proc;
    proc.start(updateBinaryAbsolutePath, {"--checkupdates"});
    if (!waitForProcess(proc))
    {
        QString errorDetails = QString::fromLocal8Bit(proc.readAllStandardError());

        if (errorDetails.toLower().contains("no updates")) {
            emit noUpdatesAvailable();
            return;
        }

        if (errorDetails.isEmpty())
            errorDetails = tr("details are unknown");

        emit updatingError(tr("Unable to check for updates (%1)").arg(errorDetails.trimmed()));
        qWarning() << "Error while checking for updates: " << errorDetails;
        return;
    }

    processCheckResults(proc.readAllStandardOutput());
}

void UpdateManager::update()
{
    bool success = QProcess::startDetached(updateBinaryAbsolutePath, {"--updater"});
    if (!success)
    {
        emit updatingError(tr("Unable to run updater application (%1). Please report this.").arg(updateBinaryAbsolutePath));
        return;
    }
    qApp->exit(0);
}

bool UpdateManager::isPlatformEligibleForUpdate() const
{
    return getDistributionType() != DistributionType::OS_MANAGED;
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

void UpdateManager::processCheckResults(const QByteArray &results)
{
    if (results.trimmed().isEmpty()) {
        emit noUpdatesAvailable();
        return;
    }

    QRegularExpression re(R"(\<update\s+([^\>]+)\>)");
    QRegularExpression versionRe(R"(version\=\"([\d\.]+)\")");
    QRegularExpression nameRe(R"(name\=\"([^\"]+)\")");

    QRegularExpressionMatchIterator reIter = re.globalMatch(results);
    QString updateNode;
    UpdateEntry theUpdate;
    QList<UpdateEntry> updates;
    while (reIter.hasNext())
    {
        updateNode = reIter.next().captured(1);
        theUpdate.version = versionRe.match(updateNode).captured(1);
        theUpdate.compontent = nameRe.match(updateNode).captured(1);
        updates << theUpdate;
    }

    if (updates.isEmpty())
        emit noUpdatesAvailable();
    else
        emit updatesAvailable(updates);
}

#endif // PORTABLE_CONFIG
