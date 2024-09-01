#include "adbmanager.h"
#include "dbandroid.h"
#include "common/utils.h"
#include <QFileInfo>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>

AdbManager::AdbManager(DbAndroid* dbAndroidPlugin) :
    QObject(dbAndroidPlugin), plugin(dbAndroidPlugin)
{
    connect(this, SIGNAL(internalDeviceListUpdate(QStringList)), this, SLOT(handleNewDeviceList(QStringList)));
    connect(this, SIGNAL(deviceDetailsChanged(QList<Device>)), this, SLOT(handleNewDetails(QList<Device>)));

    adbRunMonitor = new QTimer(this);
    connect(adbRunMonitor, SIGNAL(timeout()), this, SLOT(updateDeviceList()));
    adbRunMonitor->setSingleShot(false);
    adbRunMonitor->setInterval(1000);
    adbRunMonitor->start();
    updateDeviceList();
}

AdbManager::~AdbManager()
{
    adbRunMonitor->stop();
    updateDevicesFuture.waitForFinished();
}

const QStringList& AdbManager::getDevices(bool forceSyncUpdate)
{
    if (forceSyncUpdate)
        syncDeviceListUpdate();

    return currentDeviceList;
}

AdbManager::Device AdbManager::getDetails(const QString& deviceId)
{
    if (!currentDeviceDetails.contains(deviceId))
    {
        AdbManager::Device device;
        device.id = deviceId;
        return device;
    }

    return currentDeviceDetails[deviceId];
}

QList<AdbManager::Device> AdbManager::getDeviceDetails()
{
    return currentDeviceDetails.values();
}

QHash<QString, QPair<int, int>> AdbManager::getForwards()
{
    QHash<QString, QPair<int, int>> forwards;
    QString stdOut;
    if (!exec(QStringList({"forward", "--list"}), &stdOut))
        return forwards;

    QRegularExpression re("(.*)\\s+tcp:(\\d+)\\s+tcp:(\\d+)");
    QRegularExpressionMatch match;
    QPair<int, int> forward;
    QStringList lines = stdOut.split("\n");
    for (const QString& line : lines)
    {
        match = re.match(line);
        if (!match.hasMatch())
            continue;

        forward.first = match.captured(2).toInt();
        forward.second = match.captured(3).toInt();
        forwards[match.captured(1)] = forward;
    }

    return forwards;
}

int AdbManager::makeForwardFor(const QString& device, int targetPort)
{
    static_qstring(portTpl, "tcp:%1");

    QHash<QString, QPair<int, int>> forwards = getForwards();
    if (forwards.contains(device) && forwards[device].second == targetPort)
        return forwards[device].first;

    int localPort = targetPort;
    QStringList args = QStringList({"-s", device, "forward"});
    args << portTpl.arg(localPort);
    args << portTpl.arg(targetPort);

    int tryCount = 0;
    QString stdOut;
    bool res;
    while (!(res = exec(args, &stdOut)) && tryCount++ < 3)
    {
        localPort = rand(1025, 65000);
        args.replace(3, portTpl.arg(localPort));
    }

    if (!res)
        return -1;

    return localPort;
}

QString AdbManager::findAdb()
{
    QStringList candidates;
#ifdef Q_OS_WIN32
    candidates << "adb.exe";
#endif

#ifdef Q_OS_MACX
    candidates << (QDir::homePath() + "/Library/Android/sdk/platform-tools/adb");
#endif

#ifdef Q_OS_UNIX
    candidates << "adb" << "./adb";

    QProcess locate;
    locate.start("locate", QStringList({"adb"}));
    if (waitForProc(locate, true))
    {
        QFileInfo fi;
        QStringList locateLines = decode(locate.readAllStandardOutput()).split("\n");
        for (const QString& filePath : locateLines)
        {
            fi.setFile(filePath);
            if (fi.fileName() != "adb" || !fi.isReadable() || !fi.isExecutable())
                continue;

            candidates << filePath;
        }
    }
#endif

    QString fullPath;
    for (const QString& path : candidates)
    {
        fullPath = QDir::cleanPath(path);
        if (testAdb(fullPath, true))
            return fullPath;
    }

#ifdef Q_OS_WIN32
    if (testAdb("adb.exe", true))
        return "adb.exe";

    static_qstring(winAdbPath, "/../Android/sdk/platform-tools/adb.exe");
    for (const QString& path : QStandardPaths::standardLocations(
#if QT_VERSION >= 0x050400
             QStandardPaths::AppLocalDataLocation
#else
             QStandardPaths::GenericDataLocation
#endif
             ))
    {
        fullPath = QDir::cleanPath(path + winAdbPath);
        if (testAdb(fullPath, true))
            return fullPath;
    }
#endif

    return QString();
}

bool AdbManager::testCurrentAdb()
{
    return testAdb(plugin->getCurrentAdb(), false);
}

bool AdbManager::testAdb(const QString& adbPath, bool quiet)
{
    if (adbPath.isEmpty())
        return false;

    QProcess adbApp;
    adbApp.start(adbPath, QStringList({"version"}));
    if (!waitForProc(adbApp, quiet))
        return false;

    QString verStr = decode(adbApp.readAllStandardOutput());
    bool res = verStr.startsWith("Android Debug Bridge", Qt::CaseInsensitive);
    if (!res && !quiet)
        qWarning() << "Adb binary correct, but its version string is incorrect:" << verStr;

    return res;
}

bool AdbManager::execBytes(const QStringList& arguments, QByteArray* stdOut, QByteArray* stdErr, bool forceSafe)
{
    if (!ensureAdbRunning())
        return false;

    QProcess proc;
    if (forceSafe || arguments.join(" ").size() > 800)
    {
        if (!execLongCommand(arguments, proc, stdErr))
            return false;
    }
    else
    {
        proc.start(plugin->getCurrentAdb(), arguments);
        if (!waitForProc(proc, false))
            return false;
    }

    if (stdOut)
        *stdOut = proc.readAllStandardOutput();

    if (stdErr)
        *stdErr = proc.readAllStandardError();

    return true;
}

bool AdbManager::waitForProc(QProcess& proc, bool quiet)
{
    if (!proc.waitForFinished(-1))
    {
        if (!quiet)
            qDebug() << "DbAndroid QProcess timed out.";

        return false;
    }

    if (proc.exitStatus() == QProcess::CrashExit)
    {
        if (!quiet)
        {
            qDebug() << "DbAndroid QProcess finished by crashing.";
            qDebug() << proc.readAllStandardOutput() << proc.readAllStandardError();
        }

        return false;
    }

    if (proc.exitCode() != 0)
    {
        if (!quiet)
        {
            qDebug() << "DbAndroid QProcess finished with code:" << proc.exitCode();
            qDebug() << proc.readAllStandardOutput() << proc.readAllStandardError();
        }

        return false;
    }

    return true;
}

bool AdbManager::ensureAdbRunning()
{
    if (!plugin->isAdbValid())
        return false;

    QProcess adbApp;
    adbApp.start(plugin->getCurrentAdb(), QStringList({"start-server"}));
    if (!waitForProc(adbApp, false))
        return false;

    return true;
}

bool AdbManager::exec(const QStringList& arguments, QString* stdOut, QString* stdErr, bool forceSafe)
{
    QByteArray* out = stdOut ? new QByteArray() : nullptr;
    QByteArray* err = stdErr ? new QByteArray() : nullptr;
    bool res = execBytes(arguments, out, err, forceSafe);

    if (stdOut)
    {
        *stdOut = decode(*out);
        delete out;
    }

    if (stdErr)
    {
        *stdErr = decode(*err);
        delete err;
    }

    return res;
}

QByteArray AdbManager::encode(const QString& input)
{
    return input.toUtf8();
}

QString AdbManager::decode(const QByteArray& input)
{
    return QString::fromUtf8(input);
}

bool AdbManager::execLongCommand(const QStringList& arguments, QProcess& proc, QByteArray* stdErr)
{
    // Take off initial arguments from ADB, store it to use with "push".
    QStringList primaryArguments;
    QStringList args = arguments;

    while (args.first() != "shell")
        primaryArguments << args.takeFirst();

    args.removeFirst(); // remove the shell itself

    // Escape remaining arguments for the script
    QString cmd = " '" + args.replaceInStrings("'", "'\\''").join("' '") + "'";

    // Now, the temporary file for the script
    QTemporaryFile tmpFile("SQLiteStudio-XXXXXX.sh");
    if (!tmpFile.open())
    {
        if (stdErr)
            *stdErr = encode(QString("Could not create temporary file: %1 (%2)").arg(tmpFile.fileName(), tmpFile.errorString()));

        return false;
    }

    tmpFile.write(cmd.toUtf8());
    tmpFile.close();

    // Push the file
    args = primaryArguments;
    args << "push" << tmpFile.fileName() << "/data/local/tmp";
    proc.start(plugin->getCurrentAdb(), args);
    if (!waitForProc(proc, false))
        return false;

    QString remoteFile = ("/data/local/tmp/" + QFileInfo(tmpFile.fileName()).fileName());

    // Execute the file
    args = primaryArguments;
    args << "shell" << "sh" << remoteFile;
    proc.start(plugin->getCurrentAdb(), args);
    if (!waitForProc(proc, false))
        return false;

    // Delete the file from device
    args = primaryArguments;
    args << "shell" << "rm" << remoteFile;
    QProcess localProc;
    localProc.start(plugin->getCurrentAdb(), args);
    if (!waitForProc(localProc, false))
    {
        // Not a critical issue...
        qWarning() << "Could not clean up execution script from the device: " << remoteFile << "\nDetails:\n"
                   << localProc.readAllStandardOutput() << "\n" << localProc.readAllStandardError();
    }

    return true;
}

QStringList AdbManager::getDevicesInternal(bool emitSignal)
{
    QStringList devices;
    QString stdOut;
    if (!exec(QStringList({"devices"}), &stdOut))
    {
        if (emitSignal)
            emit internalDeviceListUpdate(devices);

        return devices;
    }

    QRegularExpression re("(.*)\\s+device$");
    QRegularExpressionMatch match;
    QStringList lines = stdOut.split("\n");
    for (const QString& line : lines)
    {
        match = re.match(line.trimmed());
        if (!match.hasMatch())
            continue;

        devices << match.captured(1).trimmed();
    }

    if (emitSignal)
        emit internalDeviceListUpdate(devices);

    return devices;
}

void AdbManager::syncDeviceListUpdate()
{
    currentDeviceList = getDevicesInternal(false);
    updateDetails(currentDeviceList);
}

void AdbManager::updateDetails(const QStringList& devices)
{
    QString stdOut;
    QList<Device> detailList;
    for (const QString& deviceId : devices)
    {
        Device deviceDetails;
        deviceDetails.id = deviceId;
        if (exec(QStringList({"-s", deviceId, "shell", "getprop", "ro.product.manufacturer"}), &stdOut))
            deviceDetails.fullName = stdOut.trimmed();
        else
            qWarning() << "Could not read brand for device" << deviceId;

        if (exec(QStringList({"-s", deviceId, "shell", "getprop", "ro.product.model"}), &stdOut))
            deviceDetails.fullName += " " + stdOut.trimmed();
        else
            qWarning() << "Could not read brand for device" << deviceId;

        deviceDetails.fullName = deviceDetails.fullName.trimmed();
        detailList << deviceDetails;
    }

    emit deviceDetailsChanged(detailList);
}

void AdbManager::updateDeviceList()
{
    if (!plugin->isAdbValid())
        return;

#if QT_VERSION >= 0x060000
    updateDevicesFuture = QtConcurrent::run(&AdbManager::getDevicesInternal, this, true);
#else
    updateDevicesFuture = QtConcurrent::run(this, &AdbManager::getDevicesInternal, true);
#endif
}

void AdbManager::handleNewDeviceList(const QStringList& devices)
{
    if (currentDeviceList == devices)
        return;

    currentDeviceList = devices;
    runInThread([=]{ updateDetails(devices); });

    emit deviceListChanged(devices);
}

void AdbManager::handleNewDetails(const QList<AdbManager::Device>& devices)
{
    currentDeviceDetails.clear();
    for (Device device : devices)
        currentDeviceDetails[device.id] = device;
}
