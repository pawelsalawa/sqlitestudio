#include "dbandroidshellconnection.h"
#include "adbmanager.h"
#include "dbandroid.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include "csvserializer.h"
#include <QMutexLocker>

const CsvFormat DbAndroidShellConnection::CSV_FORMAT = CsvFormat(",", "\r\n", true, true);

DbAndroidShellConnection::DbAndroidShellConnection(DbAndroid* plugin, const QString& deviceName, QObject* parent) :
    DbAndroidConnection(parent), plugin(plugin)
{
    this->adbManager = plugin->getAdbManager();
    this->creationDeviceName = deviceName;
    connect(adbManager, SIGNAL(deviceListChanged(QStringList)), this, SLOT(checkForDisconnection(QStringList)));
}

DbAndroidShellConnection::~DbAndroidShellConnection()
{

}

bool DbAndroidShellConnection::connectToAndroid(const DbAndroidUrl& url)
{
    if (url.getMode() != DbAndroidMode::SHELL)
        return false;

    if (!adbManager->getDevices().contains(url.getDevice()))
    {
        notifyWarn(tr("Cannot connect to device %1, because it's not visible from your computer.").arg(url.getDevice()));
        return false;
    }

    // Check if application is correct
    if (url.getApplication().isEmpty())
    {
        qCritical() << "Tried to connect to an empty application in DbAndroidShellConnection::connectToAndroid()";
        return false;
    }

    QString stdOut;
    bool res = adbManager->exec(QStringList({"-s", url.getDevice(), "shell", "run-as", url.getApplication(), "ls"}), &stdOut);
    if (!res)
    {
        notifyWarn(tr("Cannot connect to device %1, because the application %2 doesn't seem to be installed on the device.").arg(url.getDevice(), url.getApplication()));
        return false;
    }

    QMutexLocker lock(&appOkMutex);
    appOkay = true;
    if (stdOut.startsWith("run-as:"))
    {
        appOkay = false;
        qWarning() << "Cannot connect to device" << url.getDevice() << "/" << url.getApplication() << "\nDetails:\n" << stdOut.trimmed();
        notifyWarn(tr("Cannot connect to device %1, because the application %2 is not debuggable.")
                   .arg(url.getDevice(), url.getApplication()));
        return false;
    }

    // Check if sqlite3 is available
    res = adbManager->exec(QStringList({"-s", url.getDevice(), "shell", "sqlite3", "--version"}), &stdOut);
    if (!res || !stdOut.startsWith("3."))
    {
        notifyWarn(tr("Cannot connect to device %1, because '%2' command doesn't seem to be available on the device.").arg(url.getDevice(), "sqlite3"));
        return false;
    }

    // Check if databases directory exists
    res = adbManager->exec(QStringList({"-s", url.getDevice(), "shell", "run-as", url.getApplication(), "ls", "databases"}));
    if (!res)
    {
        // Doesn't exist. Create if possible.
        res = adbManager->exec(QStringList({"-s", url.getDevice(), "shell", "run-as", url.getApplication(), "mkdir", "databases"}));
        if (!res)
        {
            notifyWarn(tr("Cannot connect to device %1, because '%2' database cannot be accessed on the device.").arg(url.getDevice(), "sqlite3"));
            return false;
        }
    }

    // Try to connect to target database.
    connectionUrl = url;
    connected = true;

    ExecutionResult response = executeQuery("select sqlite_version()");
    if (response.wasError)
    {
        disconnectFromAndroid();
        notifyWarn(tr("Cannot connect to device %1, because '%2' database cannot be accessed on the device. Details: %3")
                   .arg(url.getDevice(), "sqlite3", response.errorMsg));
        return false;
    }

    return true;
}

void DbAndroidShellConnection::disconnectFromAndroid()
{
    connectionUrl = DbAndroidUrl();
    connected = false;
}

bool DbAndroidShellConnection::isConnected() const
{
    return connected;
}

QString DbAndroidShellConnection::getDbName() const
{
    return connectionUrl.getDbName();
}

QStringList DbAndroidShellConnection::getDbList()
{
    QMutexLocker lock(&appOkMutex);
    appOkay = true;
    QString out;
    bool res = adbManager->exec(QStringList({"-s", connectionUrl.getDevice(), "shell", "run-as", connectionUrl.getApplication(), "ls", "databases"}), &out);
    if (!res)
        return QStringList();

    if (out.startsWith("run-as:")) // means error
    {
        appOkay = false;
        notifyWarn(tr("Cannot get list of databases for application %1. Details: %2").arg(connectionUrl.getApplication(), out.trimmed()));
        qWarning() << "DbAndroidShellConnection::getDbList():" << out;
        return QStringList();
    }

    QStringList finalList;
    for (const QString& dbName : out.trimmed().split("\n", Qt::SkipEmptyParts))
    {
        if (dbName.trimmed().endsWith("-journal"))
            continue;

        finalList << dbName.trimmed();
    }

    return finalList;
}

QStringList DbAndroidShellConnection::getAppList()
{
    QString out;
    bool res = adbManager->exec(QStringList({"-s", creationDeviceName, "shell", "pm list packages -3"}), &out);
    if (!res)
        return QStringList();

    QStringList appList;
    for (const QString& line : out.trimmed().split("\n", Qt::SkipEmptyParts))
    {
        if (!line.startsWith("package:"))
            continue; // some other message

        appList << line.mid(8).trimmed(); // skip "package:" prefix
    }

    return appList;
}

bool DbAndroidShellConnection::isAppOkay() const
{
    QMutexLocker lock(&appOkMutex);
    return appOkay;
}

bool DbAndroidShellConnection::deleteDatabase(const QString& dbName)
{
    return adbManager->exec(QStringList({"-s", connectionUrl.getDevice(), "shell", "run-as", connectionUrl.getApplication(), "rm", "-f", "databases/" + dbName, "databases/" + dbName + "-journal"}));
}

DbAndroidConnection::ExecutionResult DbAndroidShellConnection::executeQuery(const QString& query)
{
    const static QStringList stdArguments = QStringList({"-s", connectionUrl.getDevice(), "shell", "run-as", "", "sqlite3", "-csv", "-separator", ",", "-batch", "-header"});

    // Prepare usual arguments
    QStringList args = stdArguments;
    args.replace(4, connectionUrl.getApplication());
    args << "databases/" + connectionUrl.getDbName();
    args << AdbManager::encode(query);

    // In case of SELECT we want to union typeof() for all columns first, then original query
    bool isSelect = false;
    getQueryAccessMode(query, &isSelect);
    QStringList columnNames;
    bool firstHalfForTypes = false;
    if (isSelect)
    {
        columnNames = findColumns(args, query);
        if (columnNames.size() > 0)
        {
            firstHalfForTypes = true;
            args.removeLast();
            args << AdbManager::encode(appendTypeQueryPart(query, columnNames));
        }
    }

    // Execute query and handle results
    DbAndroidConnection::ExecutionResult results;
    QByteArray out;
    QByteArray err;
    bool res = adbManager->execBytes(args, &out, &err, true);
    if (!res)
    {
        results.wasError = true;
        results.errorMsg = tr("Could not execute query on database '%1': %2").arg(connectionUrl.getDbName(), AdbManager::decode(err));
        return results;
    }

    if (out.startsWith("run-as:")) // means error
    {
        results.wasError = true;
        results.errorMsg = tr("Could not execute query on database '%1': %2").arg(connectionUrl.getDbName(), AdbManager::decode(out).trimmed());
        return results;
    }


    QList<QList<QByteArray>> deserialized = CsvSerializer::deserialize(out, CSV_FORMAT);
    if (deserialized.size() == 0)
        return results; // no results

    extractResultData(deserialized, firstHalfForTypes, results);
    return results;
}

QStringList DbAndroidShellConnection::findColumns(const QStringList& originalArgs, const QString& query)
{
    static_qstring(colQueryTpl, "SELECT * FROM (%1) LIMIT 1");

    QStringList tmpArgs = originalArgs;
    QString tmpQuery = query.trimmed();
    if (tmpQuery.endsWith(";"))
        tmpQuery.chop(1);

    tmpQuery = colQueryTpl.arg(tmpQuery);

    tmpArgs.removeLast();
    tmpArgs << tmpQuery;

    QString out;
    QString err;
    bool res = adbManager->exec(tmpArgs, &out, &err, true);
    if (!res)
    {
        qCritical() << "Error querying columns in DbAndroidShellConnection::findColumns(): " << out << "\n" << err;
        return QStringList();
    }

    QList<QStringList> deserialized = CsvSerializer::deserialize(out, CSV_FORMAT);
    if (deserialized.size() < 1)
    {
        // There will be no results.
        return QStringList();
    }

    return deserialized.first();
}

QString DbAndroidShellConnection::appendTypeQueryPart(const QString& query, const QStringList& columnNames)
{
    static_qstring(typeTpl, "typeof(%1)");
    static_qstring(hexTpl, "hex(%1) AS %1");
    static_qstring(finalQueryTpl, "SELECT %3 FROM (%2) UNION ALL SELECT %1 FROM (%2)");

    QString tmpQuery = query.trimmed();
    if (tmpQuery.endsWith(";"))
        tmpQuery.chop(1);

    QStringList hexColumns;
    QStringList typeColumns;
    QString wrappedCol;
    for (const QString& colName : columnNames)
    {
        wrappedCol = wrapObjIfNeeded(colName);
        typeColumns << typeTpl.arg(wrappedCol);
        hexColumns << hexTpl.arg(wrappedCol);
    }

    return finalQueryTpl.arg(typeColumns.join(", "), tmpQuery, hexColumns.join(", "));
}

void DbAndroidShellConnection::extractResultData(const QList<QList<QByteArray>>& deserialized, bool firstHalfForTypes, DbAndroidConnection::ExecutionResult& results)
{
    for (const QByteArray& cell : deserialized.first())
        results.resultColumns << AdbManager::decode(cell);

    QList<QList<QByteArray>> data = deserialized.mid(1); // first row are column names
    QList<QList<QByteArray>> types;
    if (firstHalfForTypes)
    {
        types = data.mid(data.size() / 2);
        data = data.mid(0, data.size() / 2);

        QVariantList rowDataList;
        QVariantHash rowDataMap;
        QList<QByteArray> rowData;
        QList<QByteArray> rowTypes;
        QVariant value;
        for (int rowIdx = 0, totalRows = data.size(); rowIdx < totalRows; ++rowIdx)
        {
            rowData = data[rowIdx];
            rowTypes = types[rowIdx];

            rowDataList.clear();
            rowDataMap.clear();
            for (int i = 0, total = rowData.size(); i < total; ++i)
            {
                value = valueFromString(rowData[i], rowTypes[i]);
                rowDataList << value;
                rowDataMap[results.resultColumns[i]] = value;
            }
            results.resultDataList << rowDataList;
            results.resultDataMap << rowDataMap;
        }
    }
    else
    {
        QVariantList rowDataList;
        QVariantHash rowDataMap;
        for (const QList<QByteArray>& row : data)
        {
            rowDataList.clear();
            rowDataMap.clear();
            for (int i = 0, total = row.size(); i < total; ++i)
            {
                rowDataList << AdbManager::decode(row[i]);
                rowDataMap[results.resultColumns[i]] = row[i];
            }
            results.resultDataList << rowDataList;
            results.resultDataMap << rowDataMap;
        }
    }
}

QVariant DbAndroidShellConnection::valueFromString(const QByteArray& bytes, const QByteArray& type)
{
    static const QStringList types = QStringList({"null", "integer", "real", "text", "blob"});

    SqliteDataType dataType = static_cast<SqliteDataType>(types.indexOf(AdbManager::decode(type).trimmed()));
    QByteArray decodedBytes = QByteArray::fromHex(bytes);
    switch (dataType)
    {
        case SqliteDataType::BLOB:
            return decodedBytes;
        case SqliteDataType::INTEGER:
            return QString::fromLatin1(decodedBytes).toLongLong();
        case SqliteDataType::REAL:
            return QString::fromLatin1(decodedBytes).toDouble();
        case SqliteDataType::TEXT:
            return QString::fromUtf8(decodedBytes);
        case SqliteDataType::_NULL:
            break;
        case SqliteDataType::UNKNOWN:
            qCritical() << "Unknown type passed to DbAndroidShellConnection::valueFromString():" << type;
            break;
    }
    return QVariant(QString());
}

void DbAndroidShellConnection::checkForDisconnection(const QStringList& devices)
{
    if (connected && !devices.contains(connectionUrl.getDevice()))
    {
        disconnectFromAndroid();
        emit disconnected();
    }
}

