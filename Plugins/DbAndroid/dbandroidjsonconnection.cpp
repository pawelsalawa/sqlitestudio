#include "dbandroidjsonconnection.h"
#include "dbandroid.h"
#include "adbmanager.h"
#include "services/notifymanager.h"
#include "common/blockingsocket.h"
#include "db/sqlerrorcodes.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent/QtConcurrent>

DbAndroidJsonConnection::DbAndroidJsonConnection(DbAndroid* plugin, QObject *parent) :
    DbAndroidConnection(parent), plugin(plugin)
{
    socket = new BlockingSocket(this);
    adbManager = plugin->getAdbManager();
    connect(socket, SIGNAL(disconnected()), this, SLOT(handlePossibleDisconnection()));
}

DbAndroidJsonConnection::~DbAndroidJsonConnection()
{
    cleanUp();
}

bool DbAndroidJsonConnection::connectToAndroid(const DbAndroidUrl& url)
{
    if (isConnected())
    {
        qWarning() << "Already connected while calling DbAndroidConnection::connect().";
        return false;
    }

    dbUrl = url;
    mode = url.getMode();

    switch (mode)
    {
        case DbAndroidMode::NETWORK:
            return connectToNetwork();
        case DbAndroidMode::USB:
            return connectToDevice();
        case DbAndroidMode::SHELL:
            qCritical() << "SHELL mode encountered in DbAndroidJsonConnection";
            break;
        case DbAndroidMode::null:
            qCritical() << "Null mode encountered in DbAndroidJsonConnection";
            break;
    }

    qCritical() << "Invalid Android db mode while connecting:" << static_cast<int>(mode);
    return false;
}

void DbAndroidJsonConnection::disconnectFromAndroid()
{
    socket->disconnectFromHost();
    connectedState = false;
}

bool DbAndroidJsonConnection::isConnected() const
{
    if (!socket)
        return false;

    return connectedState;
}

QByteArray DbAndroidJsonConnection::send(const QByteArray& data)
{
    QByteArray bytes = sizeToBytes(data.size());
    bytes.append(data);
    return sendBytes(bytes);
}

QString DbAndroidJsonConnection::getDbName() const
{
    return dbUrl.getDbName();
}

QByteArray DbAndroidJsonConnection::sendBytes(const QByteArray& data)
{
    //qDebug() << "Sending" << data;
    bool success = socket->send(data);
    if (!success)
    {
        qCritical() << "Error writing bytes to Android socket:" << socket->getErrorText();
        return QByteArray();
    }

    QByteArray sizeBytes = socket->read(4, 5000, &success);
    if (!success)
    {
        qCritical() << "Error reading response size from Android socket:" << socket->getErrorText();
        return QByteArray();
    }

    qint32 size = bytesToSize(sizeBytes);
    QByteArray responseBytes = socket->read(size, 5000, &success);
    if (!success)
    {
        qCritical() << "Error reading response from Android socket:" << socket->getErrorText();
        return QByteArray();
    }
    //qDebug() << "Received" << responseBytes;
    return responseBytes;
}

void DbAndroidJsonConnection::handleSocketError()
{
    qWarning() << "Blocking socket error in Android connection:" << socket->getErrorText();
    handlePossibleDisconnection();
}

void DbAndroidJsonConnection::handlePossibleDisconnection()
{
    if (connectedState && !socket->isConnected())
    {
        connectedState = false;
        emit disconnected();
    }
}

QByteArray DbAndroidJsonConnection::sizeToBytes(qint32 size)
{
    QByteArray bytes;
    for (int i = 0; i < 4; i++)
        bytes.append((size >> (8*i)) & 0xff);

    return bytes;
}

qint32 DbAndroidJsonConnection::bytesToSize(const QByteArray& bytes)
{
    int size = (((unsigned char)bytes[3]) << 24) |
            (((unsigned char)bytes[2]) << 16) |
            (((unsigned char)bytes[1]) << 8) |
            ((unsigned char)bytes[0]);

    return size;
}

QVariant DbAndroidJsonConnection::convertJsonValue(const QJsonValue& value)
{
    if (value.isArray())
    {
        // BLOB
        QJsonArray blobContainer = value.toArray();
        if (blobContainer.size() < 1)
        {
            qCritical() << "Invalid blob value from Android - empty array.";
            return QByteArray();
        }

        return convertBlob(blobContainer.first().toString());
    }

    // Regular value
    return value.toVariant();
}

bool DbAndroidJsonConnection::connectToNetwork()
{
    if (!dbUrl.isHostValid())
        return false;

    return connectToTcp(dbUrl.getHost(), dbUrl.getPort());
}

bool DbAndroidJsonConnection::connectToDevice()
{
    if (!plugin->isAdbValid())
        return false;

    if (!plugin->getAdbManager()->getDevices().contains(dbUrl.getDevice()))
    {
        notifyWarn(tr("Cannot connect to device %1, because it's not visible from your computer.").arg(dbUrl.getDevice()));
        return false;
    }

    int localPort = plugin->getAdbManager()->makeForwardFor(dbUrl.getDevice(), dbUrl.getPort());
    if (localPort < 0)
    {
        notifyError(tr("Failed to create port forwarding for device %1 for port %2.")
                    .arg(dbUrl.getDevice(), QString::number(dbUrl.getPort())));
        return false;
    }

    return connectToTcp("127.0.0.1", localPort);
}

bool DbAndroidJsonConnection::connectToTcp(const QString& ip, int port)
{
    bool success = socket->connectToHost(ip, port);
    if (!success)
    {
        qWarning() << "Could not connect to network host for Android DB:" << ip << ":" << port <<  ", details:" << socket->getErrorText();
        notifyWarn(tr("Could not connect to network host: %1:%2").arg(ip, QString::number(port)));
        return false;
    }

    connectedState = true;

    // Authenticate
    QString pass = dbUrl.getPassword();
    if (!pass.isEmpty())
    {
        static_qstring(passPharse, "{auth:\"%1\"}");
        QByteArray response = send(passPharse.arg(pass.replace("\"", "\\\"")).toUtf8());
        if (response != PASS_RESPONSE_OK)
        {
            notifyWarn(tr("Cannot connect to %1:%2, because password is invalid.").arg(ip, QString::number(port)));
            handleConnectionFailed();
            return false;
        }
    }

    return true;
}

void DbAndroidJsonConnection::handleConnectionFailed()
{
    connectedState = false;
    socket->disconnectFromHost();
}

void DbAndroidJsonConnection::cleanUp()
{
    disconnectFromAndroid();
    safe_delete(socket);
}

QStringList DbAndroidJsonConnection::getDbList()
{
    if (!isConnected())
    {
        qWarning() << "Called DbAndroidJsonConnection::getDbList() on closed connection.";
        return QStringList();
    }

    QByteArray result = send(LIST_CMD);
    return handleDbListResult(result);
}

QStringList DbAndroidJsonConnection::getAppList()
{
    return QStringList();
}

bool DbAndroidJsonConnection::isAppOkay() const
{
    return true;
}

QStringList DbAndroidJsonConnection::handleDbListResult(const QByteArray& results)
{
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(results, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        qCritical() << "Error while parsing response from Android:" << jsonError.errorString();
        return QStringList();
    }

    QJsonObject responseObject = jsonResponse.object();
    if (responseObject.contains("generic_error"))
    {
        qCritical() << "Generic error from Android:" << responseObject["generic_error"].toInt();
        return QStringList();
    }

    if (!responseObject.contains("list"))
    {
        qCritical() << "Missing 'list' in response from Android.";
        return QStringList();
    }

    QStringList dbNames;
    for (const QVariant& name : responseObject["list"].toArray().toVariantList())
        dbNames << name.toString();

    return dbNames;
}

bool DbAndroidJsonConnection::deleteDatabase(const QString& dbName)
{
    if (!isConnected())
    {
        qWarning() << "Called DbAndroidConnection::deleteDatabase() on closed database.";
        return false;
    }

    QByteArray result = send(QString(DELETE_DB_CMD).arg(dbName).toUtf8());
    return handleStdResult(result);
}

DbAndroidConnection::ExecutionResult DbAndroidJsonConnection::executeQuery(const QString& query)
{
    DbAndroidConnection::ExecutionResult executionResults;
    if (!isConnected())
    {
        executionResults.wasError = true;
        executionResults.errorMsg = tr("Unable to execute query on Android device (connection was closed): %1").arg(query);
        return executionResults;
    }

    QJsonDocument json = wrapQueryInJson(query);
    QByteArray responseBytes = send(json.toJson(QJsonDocument::Compact));

    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseBytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        executionResults.wasError = true;
        executionResults.errorMsg = tr("Error while parsing response from Android: %1").arg(jsonError.errorString());
        return executionResults;
    }

    QJsonObject responseObject = jsonResponse.object();
    if (responseObject.contains("generic_error"))
    {
        executionResults.wasError = true;
        executionResults.errorMsg = tr("Generic error from Android: %1").arg(responseObject["generic_error"].toInt());
        return executionResults;
    }

    if (responseObject.contains("error_code"))
    {
        executionResults.errorCode = responseObject["error_code"].toInt();
        executionResults.errorMsg = responseObject["error_message"].toString();
        return executionResults;
    }

    if (!responseObject.contains("columns"))
    {
        executionResults.wasError = true;
        executionResults.errorMsg = tr("Missing 'columns' in response from Android.");
        return executionResults;
    }

    if (!responseObject.contains("data"))
    {
        executionResults.wasError = true;
        executionResults.errorMsg = tr("Missing 'columns' in response from Android.");
        return executionResults;
    }

    for (const QVariant& col : responseObject["columns"].toArray().toVariantList())
        executionResults.resultColumns << col.toString();

    QJsonArray jsonRows = responseObject["data"].toArray();
    QJsonObject jsonRow;
    QJsonValue jsonValue;
    QVariantHash rowAsMap;
    QVariantList rowAsList;
    QVariant cellValue;
    for (int i = 0, total = jsonRows.size(); i < total; ++i)
    {
        jsonRow = jsonRows[i].toObject();
        for (const QString& colName : executionResults.resultColumns)
        {
            if (!jsonRow.contains(colName))
            {
                executionResults.wasError = true;
                executionResults.errorMsg = tr("Response from Android has missing data for column '%1' in row %2.").arg(colName, QString::number(i+1));
                return executionResults;
            }

            jsonValue = jsonRow[colName];
            cellValue = convertJsonValue(jsonValue);
            rowAsMap[colName] = cellValue;
            rowAsList << cellValue;
        }

        executionResults.resultDataMap << rowAsMap;
        executionResults.resultDataList << rowAsList;

        rowAsMap.clear();
        rowAsList.clear();
    }

    return executionResults;
}

QJsonDocument DbAndroidJsonConnection::wrapQueryInJson(const QString& query)
{
    QJsonDocument doc;

    QJsonObject rootObj;
    rootObj["cmd"] = "QUERY";
    rootObj["db"] = dbUrl.getDbName();
    rootObj["query"] = query;

    doc.setObject(rootObj);
    return doc;
}

bool DbAndroidJsonConnection::handleStdResult(const QByteArray& results)
{
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(results, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        qCritical() << "Error while parsing response from Android:" << jsonError.errorString();
        return false;
    }

    QJsonObject responseObject = jsonResponse.object();
    if (responseObject.contains("generic_error"))
    {
        qCritical() << "Generic error from Android:" << responseObject["generic_error"].toInt();
        return false;
    }

    if (!responseObject.contains("result"))
    {
        qCritical() << "Missing 'result' in response from Android.";
        return false;
    }

    return (responseObject["result"].toString() == "ok");
}
