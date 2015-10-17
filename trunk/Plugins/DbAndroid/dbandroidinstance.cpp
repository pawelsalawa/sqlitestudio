#include "dbandroidconnection.h"
#include "dbandroidinstance.h"
#include "sqlqueryandroid.h"
#include "db/sqlerrorcodes.h"
#include "common/unused.h"
#include "dbandroid.h"
#include "dbandroidjsonconnection.h"
#include "dbandroidconnectionfactory.h"
#include "dbandroidurl.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

DbAndroidInstance::DbAndroidInstance(DbAndroid* plugin, const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb(name, path, connOptions), plugin(plugin)
{
    this->connOptions[SchemaResolver::USE_SCHEMA_CACHING] = true;
}

DbAndroidInstance::~DbAndroidInstance()
{
    closeInternal();
}

SqlQueryPtr DbAndroidInstance::prepare(const QString& query)
{
    return SqlQueryPtr(new SqlQueryAndroid(this, connection, query));
}

QString DbAndroidInstance::getTypeLabel()
{
    return plugin->getLabel();
}

bool DbAndroidInstance::deregisterFunction(const QString& name, int argCount)
{
    // Unsupported by native Android driver
    UNUSED(name);
    UNUSED(argCount);
    return true;
}

bool DbAndroidInstance::registerScalarFunction(const QString& name, int argCount)
{
    // Unsupported by native Android driver
    UNUSED(name);
    UNUSED(argCount);
    return true;
}

bool DbAndroidInstance::registerAggregateFunction(const QString& name, int argCount)
{
    // Unsupported by native Android driver
    UNUSED(name);
    UNUSED(argCount);
    return true;
}

bool DbAndroidInstance::initAfterCreated()
{
    version = 3;
    return AbstractDb::initAfterCreated();
}

bool DbAndroidInstance::isOpenInternal()
{
    return (connection && connection->isConnected());
}

void DbAndroidInstance::interruptExecution()
{
    // Unsupported by native Android driver
}

QString DbAndroidInstance::getErrorTextInternal()
{
    return errorText;
}

int DbAndroidInstance::getErrorCodeInternal()
{
    return errorCode;
}

bool DbAndroidInstance::openInternal()
{
    connection = createConnection();
    bool res = connection->connectToAndroid(DbAndroidUrl(path));
    if (!res)
    {
        safe_delete(connection);
    }
    else
    {
        connect(connection, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
    }

    return res;
}

bool DbAndroidInstance::closeInternal()
{
    if (!connection)
        return false;

    disconnect(connection, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
    connection->disconnectFromAndroid();
    safe_delete(connection);
    return true;
}

bool DbAndroidInstance::registerCollationInternal(const QString& name)
{
    // Unsupported by native Android driver
    UNUSED(name);
    return true;
}

bool DbAndroidInstance::deregisterCollationInternal(const QString& name)
{
    // Unsupported by native Android driver
    UNUSED(name);
    return true;
}

DbAndroidConnection* DbAndroidInstance::createConnection()
{
    DbAndroidUrl url(path);
    if (!url.isValid(false))
        return nullptr;

    return plugin->getConnectionFactory()->create(url, this);
}

void DbAndroidInstance::handleDisconnected()
{
    safe_delete(connection);
    notifyWarn(tr("Connection with Android database '%1' lost.").arg(getName()));
    emit disconnected();
}
