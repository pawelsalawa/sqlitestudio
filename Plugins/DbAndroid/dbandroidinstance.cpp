#include "dbandroidconnection.h"
#include "dbandroidinstance.h"
#include "parser/ast/sqlitepragma.h"
#include "sqlqueryandroid.h"
#include "db/sqlerrorcodes.h"
#include "dbandroid.h"
#include "dbandroidjsonconnection.h"
#include "dbandroidconnectionfactory.h"
#include "dbandroidurl.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "db/dbsqlite3.h"
#include "parser/parser.h"
#include "common/utils.h"
#include "../Letos/core/common/utils_sql.h"
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

    static_qstring(zeroResultsTpl, "select * from letos_inspect_columns(?);");
    QString queryStr = query.trimmed();
    if (queryStr.endsWith(";"))
        queryStr.chop(1);

    SqlQueryPtr results = exec(zeroResultsTpl, queryStr);
    if (results->isError())
    {
        qWarning() << "Could not execute query for providing columnsForQuery from DbAndroid:" << query
                   << ". The error was:" << results->getErrorText();
        return QList<AliasedColumn>();
    }

    QList<AliasedColumn> columns;
    AliasedColumn column;
    for (SqlResultsRowPtr row : results->getAll())
    {
        column.setDatabase(row->value("database").toString());
        column.setTable(row->value("table_name").toString());
        column.setColumn(row->value("column_name").toString());
        column.setAlias(row->value("alias").toString());
        columns << column;
    }
    return columns;
}

SqlQueryPtr DbAndroidInstance::prepare(const QString& query) const
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
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    return true;
}

bool DbAndroidInstance::registerScalarFunction(const QString& name, int argCount, bool deterministic)
{
    // Unsupported by native Android driver
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    Q_UNUSED(deterministic);
    return true;
}

bool DbAndroidInstance::registerAggregateFunction(const QString& name, int argCount, bool deterministic)
{
    // Unsupported by native Android driver
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    Q_UNUSED(deterministic);
    return true;
}

bool DbAndroidInstance::registerAggregateWindowFunction(const QString& name, int argCount, bool deterministic)
{
    // Unsupported by native Android driver
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    Q_UNUSED(deterministic);
    return true;
}

bool DbAndroidInstance::initAfterCreated()
{
    version = 3;
    return AbstractDb::initAfterCreated();
}

bool DbAndroidInstance::loadExtension(const QString& filePath, const QString& initFunc)
{
    Q_UNUSED(filePath);
    Q_UNUSED(initFunc);
    errorCode = 1;
    errorText = tr("Android SQLite driver does not support loadable extensions.");
    return false;
}

bool DbAndroidInstance::isComplete(const QString& sql) const
{
    return DbSqlite3::complete(sql);
}

AbstractDb* DbAndroidInstance::createCloneInstance() const
{
    return new DbAndroidInstance(plugin, name, path, connOptions);
}

void DbAndroidInstance::initAfterOpen()
{
    SqlQueryPtr res;

    QString cipher = connOptions[DbAndroid::CIPHER_OPT].toString();
    if (!cipher.isEmpty())
    {
        res = exec(QString("PRAGMA cipher = '%1';").arg(cipher), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLite3MC cipher:" << res->getErrorText();
    }

    QString pragmas = connOptions[DbAndroid::PRAGMAS_OPT].toString();
    QStringList pragmaList = quickSplitQueries(pragmas);
    for (const QString& pragma : pragmaList)
    {
        QString pragmaSql = removeComments(pragma).trimmed();
        if (pragmaSql.isEmpty())
            continue;

        res = exec(pragmaSql, Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLite3MC pragma" << pragma << ":" << res->getErrorText();
    }

    QString key = connOptions[DbAndroid::PASSWORD_OPT].toString();
    if (!key.isEmpty())
    {
        res = exec(QString("PRAGMA key = '%1';").arg(escapeString(key)), Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Error while defining SQLite3MC key:" << res->getErrorText();
    }

    AbstractDb::initAfterOpen();
}

bool DbAndroidInstance::isTransactionActive() const
{
    SqlQueryPtr res = exec("BEGIN");
    if (res->isError())
        return true; // cannot begin top-level tx? Then we're in a tx already

    exec("ROLLBACK"); // there was no transaction, we need to roll back the one we started
    return false;
}

Db::TransactionState DbAndroidInstance::getTransactionState() const
{
    // Since there is no way to detect transaction state by PRAGMA or SQL function,
    // this plugin just assumes any active transation to be at WRITE level.
    // It's not ideal, but it's the best we can do.
    return isTransactionActive() ? TransactionState::WRITE : TransactionState::NONE;
}

bool DbAndroidInstance::isOpenInternal() const
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
    DbAndroidUrl dbUrl(path);
    connection = createConnection();
    bool res = connection->connectToAndroid(dbUrl);
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

bool DbAndroidInstance::closeInternal(bool walCheckpoint)
{
    if (!connection)
        return false;

    disconnect(connection, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
    if (walCheckpoint)
        exec("PRAGMA wal_checkpoint(FULL)", Flag::NO_LOCK);

    connection->disconnectFromAndroid();
    safe_delete(connection);
    return true;
}

bool DbAndroidInstance::registerCollationInternal(const QString& name)
{
    // Unsupported by native Android driver
    Q_UNUSED(name);
    return true;
}

bool DbAndroidInstance::deregisterCollationInternal(const QString& name)
{
    // Unsupported by native Android driver
    Q_UNUSED(name);
    return true;
}

DbAndroidConnection* DbAndroidInstance::createConnection()
{
    DbAndroidUrl url(path);
    if (!url.isValid(false))
        return nullptr;

    return plugin->getConnectionFactory()->create(url);
}

void DbAndroidInstance::handleDisconnected()
{
    safe_delete(connection);
    notifyWarn(tr("Connection with Android database '%1' lost.").arg(getName()));
    emit disconnected();
}
