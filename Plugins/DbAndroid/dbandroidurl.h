#ifndef DBANDROIDURL_H
#define DBANDROIDURL_H

#include "dbandroidmode.h"
#include <QString>
#include <QUrl>

class DbAndroidUrl
{
    public:
        DbAndroidUrl() = default;
        DbAndroidUrl(const DbAndroidUrl& other) = default;
        explicit DbAndroidUrl(DbAndroidMode enforcedMode);
        explicit DbAndroidUrl(const QString& path, bool obfuscatedPassword = true);
        ~DbAndroidUrl() = default;

        DbAndroidUrl& operator=(const DbAndroidUrl&) = default;
        DbAndroidUrl& operator=(DbAndroidUrl&&) = default;

        QString toUrlString(bool obfuscatedPassword = true) const;
        QUrl toUrl(bool obfuscatedPassword = true) const;
        QString getDisplayName() const;

        int getPort() const;
        void setPort(int value);

        QString getDbName() const;
        void setDbName(const QString& value);

        DbAndroidMode getMode() const;
        void setEnforcedMode(DbAndroidMode mode);
        bool isValid(bool validateConnectionIrrelevantParts = true) const;
        bool isHostValid() const;
        bool isNull() const;

        QString getPassword(bool obfuscated = false) const;
        void setPassword(const QString& value, bool obfuscated = false);

        QString getHost() const;
        void setHost(const QString& value);

        QString getDevice() const;
        void setDevice(const QString& value);

        QString getApplication() const;
        void setApplication(const QString& value);

    private:
        void parse(const QString& path, bool obfuscatedPassword = false);

        static const constexpr char* SCHEME = "android";

        DbAndroidMode enforcedMode = DbAndroidMode::null;
        QString host;
        QString device;
        int port = -1;
        QString dbName;
        QString password;
        QString application;
};

#endif // DBANDROIDURL_H
