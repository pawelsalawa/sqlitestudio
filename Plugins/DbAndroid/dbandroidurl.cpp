#include "dbandroidurl.h"
#include "common/ipvalidator.h"
#include <QStringList>

DbAndroidUrl::DbAndroidUrl(DbAndroidMode enforcedMode) :
    enforcedMode(enforcedMode)
{
}

DbAndroidUrl::DbAndroidUrl(const QString& path, bool obfuscatedPassword)
{
    parse(path, obfuscatedPassword);
}

QString DbAndroidUrl::toUrlString(bool obfuscatedPassword) const
{
    return toUrl(obfuscatedPassword).toString();
}

QUrl DbAndroidUrl::toUrl(bool obfuscatedPassword) const
{
    QUrl url;
    url.setScheme(SCHEME);
    url.setHost(host);
    url.setUserName(device);
    url.setPort(port);
    url.setPassword(getPassword(obfuscatedPassword));
    url.setPath("/" + (application.isEmpty() ? "!" : application) + "/" + dbName);
    return url;
}

QString DbAndroidUrl::getDisplayName() const
{
    if (!device.isNull())
        return device;

    return host;
}

void DbAndroidUrl::parse(const QString& path, bool obfuscatedPassword)
{
    QUrl url(path);
    if (url.scheme() != SCHEME)
        return;

    host = url.host();
    device = url.userName();
    port = url.port();

    QString urlPath = url.path();
    if (urlPath.startsWith("/"))
        urlPath = urlPath.mid(1);

    QStringList pathParts = urlPath.split("/");

    application = QString();
    if (pathParts.first() != "!")
        application = pathParts.first();

    dbName = QStringList(pathParts.mid(1)).join("/");
    if (!url.password().isEmpty())
        setPassword(url.password(), obfuscatedPassword);
    else
        setPassword(QString());
}

QString DbAndroidUrl::getApplication() const
{
    return application;
}

void DbAndroidUrl::setApplication(const QString& value)
{
    application = value;
}

QString DbAndroidUrl::getDevice() const
{
    return device;
}

void DbAndroidUrl::setDevice(const QString& value)
{
    device = value;
}

QString DbAndroidUrl::getHost() const
{
    return host;
}

void DbAndroidUrl::setHost(const QString& value)
{
    host = value;
}


QString DbAndroidUrl::getPassword(bool obfuscated) const
{
    if (obfuscated)
        return QString::fromLatin1(password.toUtf8().toHex().toBase64());

    return password;
}

void DbAndroidUrl::setPassword(const QString& value, bool obfuscated)
{
    if (obfuscated)
    {
        password = QString::fromUtf8(QByteArray::fromHex(QByteArray::fromBase64(value.toLatin1())));
        return;
    }

    password = value;
}


QString DbAndroidUrl::getDbName() const
{
    return dbName;
}

void DbAndroidUrl::setDbName(const QString& value)
{
    dbName = value;
}

DbAndroidMode DbAndroidUrl::getMode() const
{
    if (enforcedMode != DbAndroidMode::null)
        return enforcedMode;

    if (!application.isEmpty())
        return DbAndroidMode::SHELL;

    return host.isEmpty() ? DbAndroidMode::USB : DbAndroidMode::NETWORK;
}

void DbAndroidUrl::setEnforcedMode(DbAndroidMode mode)
{
    enforcedMode = mode;
}

bool DbAndroidUrl::isValid(bool validateConnectionIrrelevantParts) const
{
    if (isNull())
        return false;

    if (validateConnectionIrrelevantParts && dbName.isEmpty())
        return false;

    switch (getMode())
    {
        case DbAndroidMode::NETWORK:
        {
            if (!isHostValid())
                return false;

            if (port <= 0)
                return false;

            break;
        }
        case DbAndroidMode::USB:
        {
            if (port <= 0)
                return false;

            break;
        }
        case DbAndroidMode::SHELL:
        {
            if (validateConnectionIrrelevantParts && application.isEmpty())
                return false;

            break;
        }
        case DbAndroidMode::null:
            return false;
    }

    return true;
}

bool DbAndroidUrl::isHostValid() const
{
    return IpValidator::check(host);
}

bool DbAndroidUrl::isNull() const
{
    return host.isEmpty() && device.isEmpty();
}

int DbAndroidUrl::getPort() const
{
    return port;
}

void DbAndroidUrl::setPort(int value)
{
    port = value;
}
