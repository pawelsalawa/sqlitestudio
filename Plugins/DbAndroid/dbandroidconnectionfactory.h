#ifndef DBANDROIDCONNECTIONFACTORY_H
#define DBANDROIDCONNECTIONFACTORY_H

#include "dbandroidurl.h"

class DbAndroidConnection;
class DbAndroid;

class DbAndroidConnectionFactory
{
    public:
        explicit DbAndroidConnectionFactory(DbAndroid* plugin);

        DbAndroidConnection* create(const QString& url, QObject* parent = nullptr);
        DbAndroidConnection* create(const DbAndroidUrl& url, QObject* parent = nullptr);

    private:
        DbAndroid* plugin = nullptr;
};

#endif // DBANDROIDCONNECTIONFACTORY_H
