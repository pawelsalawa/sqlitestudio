#ifndef DBANDROIDCONNECTION_H
#define DBANDROIDCONNECTION_H

#include "dbandroidurl.h"
#include <QObject>
#include <QStringList>
#include <QHash>

class DbAndroidConnection : public QObject
{
        Q_OBJECT

    public:
        struct ExecutionResult
        {
            bool wasError = false;
            int errorCode = 0;
            QString errorMsg;
            QStringList resultColumns;
            QList<QVariantHash> resultDataMap;
            QList<QVariantList> resultDataList;
        };

        DbAndroidConnection(QObject* parent = 0) : QObject(parent) {}
        virtual ~DbAndroidConnection() {}

        virtual bool connectToAndroid(const DbAndroidUrl& url) = 0;
        virtual void disconnectFromAndroid() = 0;
        virtual bool isConnected() const = 0;
        virtual QString getDbName() const = 0;
        virtual QStringList getDbList() = 0;
        virtual QStringList getAppList() = 0;
        virtual bool isAppOkay() const = 0;
        virtual bool deleteDatabase(const QString& dbName) = 0;
        virtual ExecutionResult executeQuery(const QString& query) = 0;

    protected:
        static QByteArray convertBlob(const QString& value);

    signals:
        void disconnected();
};

#endif // DBANDROIDCONNECTION_H
