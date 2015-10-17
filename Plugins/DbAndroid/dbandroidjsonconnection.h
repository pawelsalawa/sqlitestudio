#ifndef DBANDROIDJSONCONNECTION_H
#define DBANDROIDJSONCONNECTION_H

#include "dbandroidmode.h"
#include "common/global.h"
#include "common/expiringcache.h"
#include "dbandroidconnection.h"
#include <QObject>

class DbAndroid;
class AdbManager;
class BlockingSocket;

class DbAndroidJsonConnection : public DbAndroidConnection
{
        Q_OBJECT

    public:
        DbAndroidJsonConnection(DbAndroid* plugin, QObject *parent = 0);
        ~DbAndroidJsonConnection();

        bool connectToAndroid(const DbAndroidUrl& url);
        void disconnectFromAndroid();
        bool isConnected() const;
        QByteArray send(const QByteArray& data);
        QString getDbName() const;
        QStringList getDbList();
        QStringList getAppList();
        bool isAppOkay() const;
        bool deleteDatabase(const QString& dbName);
        ExecutionResult executeQuery(const QString& query);

    private:
        QJsonDocument wrapQueryInJson(const QString& query);
        bool connectToNetwork();
        bool connectToDevice();
        bool connectToTcp(const QString& ip, int port);
        void cleanUp();
        QByteArray sendBytes(const QByteArray& data);
        void handleSocketError();
        void handleConnectionFailed();
        QStringList handleDbListResult(const QByteArray& results);
        bool handleStdResult(const QByteArray& results);

        static QByteArray sizeToBytes(qint32 size);
        static qint32 bytesToSize(const QByteArray& bytes);
        static QVariant convertJsonValue(const QJsonValue& value);

        DbAndroid* plugin = nullptr;
        AdbManager* adbManager = nullptr;
        BlockingSocket* socket = nullptr;
        DbAndroidUrl dbUrl;
        DbAndroidMode mode = DbAndroidMode::NETWORK;
        bool connectedState = false;

        static_char* PASS_RESPONSE_OK = "{\"result\":\"ok\"}";
        static_char* PING_RESPONSE_OK = "{\"result\":\"pong\"}";
        static_char* LIST_CMD = "{cmd:\"LIST\"}";
        static_char* DELETE_DB_CMD = "{cmd:\"DELETE_DB\",db:\"%1\"}";

    private slots:
        void handlePossibleDisconnection();
};

#endif // DBANDROIDJSONCONNECTION_H
