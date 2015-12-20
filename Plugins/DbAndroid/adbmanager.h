#ifndef ADBMANAGER_H
#define ADBMANAGER_H

#include <QObject>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>

class DbAndroid;
class QTimer;

class AdbManager : public QObject
{
        Q_OBJECT
    public:
        struct Device
        {
            QString id;
            QString fullName;
        };

        AdbManager(DbAndroid* plugin);
        ~AdbManager();

        const QStringList& getDevices(bool forceSyncUpdate = false);
        Device getDetails(const QString& deviceId);
        QList<Device> getDeviceDetails();
        QHash<QString,QPair<int,int>> getForwards();
        int makeForwardFor(const QString& device, int targetPort);
        QString findAdb();
        bool testCurrentAdb();
        bool testAdb(const QString& adbPath, bool quiet = false);
        bool execBytes(const QStringList& arguments, QByteArray* stdOut = nullptr, QByteArray* stdErr = nullptr, bool forceSafe =  false);
        bool exec(const QStringList& arguments, QString* stdOut = nullptr, QString* stdErr = nullptr, bool forceSafe =  false);

        static QByteArray encode(const QString& input);
        static QString decode(const QByteArray& input);

    private:
        bool execLongCommand(const QStringList& arguments, QProcess& proc, QByteArray* stdErr);
        bool waitForProc(QProcess& proc, bool quiet = false);
        bool ensureAdbRunning();
        QStringList getDevicesInternal(bool emitSignal);
        void syncDeviceListUpdate();
        void updateDetails(const QStringList& devices);

        DbAndroid* plugin;
        QTimer* adbRunMonitor = nullptr;
        QStringList currentDeviceList;
        QHash<QString,Device> currentDeviceDetails;
        QFuture<QStringList> updateDevicesFuture;

    private slots:
        void updateDeviceList();
        void handleNewDeviceList(const QStringList& devices);
        void handleNewDetails(const QList<Device>& devices);

    signals:
        void internalDeviceListUpdate(const QStringList& devices);
        void deviceListChanged(const QStringList& devices);
        void deviceDetailsChanged(const QList<Device>& details);
};

Q_DECLARE_METATYPE(QList<AdbManager::Device>)

#endif // ADBMANAGER_H
