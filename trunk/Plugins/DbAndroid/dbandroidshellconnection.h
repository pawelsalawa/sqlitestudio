#ifndef DBANDROIDSHELLCONNECTION_H
#define DBANDROIDSHELLCONNECTION_H

#include "dbandroidconnection.h"
#include "csvformat.h"

#include <QMutex>

class DbAndroid;
class AdbManager;

class DbAndroidShellConnection : public DbAndroidConnection
{
        Q_OBJECT

    public:
        DbAndroidShellConnection(DbAndroid* plugin, QObject *parent = 0);
        ~DbAndroidShellConnection();

        bool connectToAndroid(const DbAndroidUrl& url);
        void disconnectFromAndroid();
        bool isConnected() const;
        QString getDbName() const;
        QStringList getDbList();
        QStringList getAppList();
        bool isAppOkay() const;
        bool deleteDatabase(const QString& dbName);
        ExecutionResult executeQuery(const QString& query);

    private:
        enum class DataType
        {
            UNKNOWN = -1,
            _NULL = 0,
            INTEGER = 1,
            REAL = 2,
            TEXT = 3,
            BLOB = 4
        };

        QStringList findColumns(const QStringList& originalArgs, const QString& query);
        QString appendTypeQueryPart(const QString& query, const QStringList& columnNames);
        void extractResultData(const QList<QList<QByteArray> >& deserialized, bool firstHalfForTypes, ExecutionResult& results);
        QVariant valueFromString(const QByteArray& bytes, const QByteArray& type);

        DbAndroid* plugin = nullptr;
        AdbManager* adbManager = nullptr;
        bool connected = false;
        DbAndroidUrl connectionUrl;
        bool appOkay = false;
        mutable QMutex appOkMutex;

        static const CsvFormat CSV_FORMAT;

    private slots:
        void checkForDisconnection(const QStringList& devices);
};

#endif // DBANDROIDSHELLCONNECTION_H
