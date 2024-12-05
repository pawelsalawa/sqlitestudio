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
#include "db/dbsqlite3.h"
#include "parser/parser.h"
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

QList<AliasedColumn> DbAndroidInstance::columnsForQuery(const QString& query)
{
    Parser parser;
    bool res = parser.parse(query);
    if (!res)
    {
        qWarning() << "Could not parse query for providing columnsForQuery from DbAndroid:" << query;
        return QList<AliasedColumn>();
    }

    if (!isDataReturningQuery(parser.getQueries().last()->queryType))
        return QList<AliasedColumn>();

    SqlQueryPtr results = exec(query);
    if (results->isError())
    {
        qWarning() << "Could not execute query for providing columnsForQuery from DbAndroid:" << query
                   << ". The error was:" << results->getErrorText();
        return QList<AliasedColumn>();
    }

    QList<AliasedColumn> columns;
    AliasedColumn column;
    for (QString& colName : results->getColumnNames())
    {
        column.setAlias(colName);
        columns << column;
    }
    return columns;
}

SqlQueryPtr DbAndroidInstance::prepare(const QString& query)
{
    return SqlQueryPtr(new SqlQueryAndroid(this, connection, query));
}

QString DbAndroidInstance::getTypeLabel() const
{
    return plugin->getLabel();
}

QString DbAndroidInstance::getTypeClassName() const
{
    return "DbAndroidInstance";
}

bool DbAndroidInstance::deregisterFunction(const QString& name, int argCount)
{
    // Unsupported by native Android driver
    UNUSED(name);
    UNUSED(argCount);
    return true;
}

bool DbAndroidInstance::registerScalarFunction(const QString& name, int argCount, bool deterministic)
{
    // Unsupported by native Android driver
    UNUSED(name);
    UNUSED(argCount);
    UNUSED(deterministic);
    return true;
}

bool DbAndroidInstance::registerAggregateFunction(const QString& name, int argCount, bool deterministic)
{
    // Unsupported by native Android driver
    UNUSED(name);
    UNUSED(argCount);
    UNUSED(deterministic);
    return true;
}

bool DbAndroidInstance::initAfterCreated()
{
    version = 3;
    return AbstractDb::initAfterCreated();
}

bool DbAndroidInstance::loadExtension(const QString& filePath, const QString& initFunc)
{
    UNUSED(filePath);
    UNUSED(initFunc);
    errorCode = 1;
    errorText = tr("Android SQLite driver does not support loadable extensions.");
    return false;
}

bool DbAndroidInstance::isComplete(const QString& sql) const
{
    return DbSqlite3::complete(sql);
}

Db* DbAndroidInstance::clone() const
{
    return new DbAndroidInstance(plugin, name, path, connOptions);
}

bool DbAndroidInstance::isTransactionActive() const
{
    // FIXME: Since there is no way to detect active transaction by PRAGMA or SQL function,
    // this plugin cannot just test this state. It could potentially be implemented by tracking BEGIN/END keywords
    // as first keywords in executed queries, but it will require more work and testing.
    //
    // Current consiequence of negleting this is that if user uses import() function enclosed with BEGIN & END,
    // the query will fail due to already active transaction. In that case user needs to skip the BEGIN & END.
    return false;
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

bool DbAndroidInstance::flushWalInternal()
{
    // WAL mode is not fully supporeted by Android plugin.
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
