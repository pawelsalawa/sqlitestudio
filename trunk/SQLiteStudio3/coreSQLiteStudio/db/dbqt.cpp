#include "dbqt.h"
#include "dbmanager.h"
#include "utils.h"
#include "asyncqueryrunner.h"
#include "sqlresultsqt.h"
#include "sqlresultsrow.h"
#include "utils_sql.h"
#include "parser/ast/sqliteinsert.h"
#include "schemaresolver.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QThreadPool>

#include <parser/parser.h>

DbQt::DbQt(const QString &driverName, const QString &type)
    : driverName(driverName), type(type)
{
}

DbQt::~DbQt()
{
    //qDebug() << "deleting DbQt" << getName() << this;
    detachAll();
    close();
    if (db)
    {
        delete db;
        db = nullptr;
    }
    QSqlDatabase::removeDatabase(name);
}

QString DbQt::getErrorTextInternal()
{
    if (!db)
        return QString::null;

    return db->lastError().databaseText().isEmpty() ? db->lastError().driverText() : db->lastError().databaseText();
}

int DbQt::getErrorCodeInternal()
{
    if (!db)
        return 0;

    return db->lastError().number();
}

bool DbQt::deregisterFunction(const QString& name, int argCount)
{
    if (!isOpenInternal())
        return false;

    QVariant handle = db->driver()->handle();
    if (!handle.isValid())
    {
        qWarning() << "Call to deregisterFunction() on db object, but driver handle is invalid QVariant.";
        return false;
    }

    return deregisterFunction(handle, name, argCount);
}

bool DbQt::registerScalarFunction(const QString& name, int argCount)
{
    if (!isOpenInternal())
        return false;

    QVariant handle = db->driver()->handle();
    if (!handle.isValid())
    {
        qWarning() << "Call to registerScalarFunction() on db object, but driver handle is invalid QVariant.";
        return false;
    }

    return registerScalarFunction(handle, name, argCount);
}

bool DbQt::registerAggregateFunction(const QString& name, int argCount)
{
    if (!isOpenInternal())
        return false;

    QVariant handle = db->driver()->handle();
    if (!handle.isValid())
    {
        qWarning() << "Call to registerAggregateFunction() on db object, but driver handle is invalid QVariant.";
        return false;
    }

    return registerAggregateFunction(handle, name, argCount);
}

bool DbQt::openInternal()
{

    if (!db)
        return false;

    if (!db->isValid())
        return false;

    if (db->isOpen())
        return true;

    return db->open();
}

bool DbQt::closeInternal()
{
    if (!db)
        return false;

    if (!isOpenInternal())
        return false;

    db->close();
    attachedDbMap.clear();
    return true;
}

void DbQt::interruptExecution()
{
    if (!isOpenInternal())
        return;

    QVariant handle = db->driver()->handle();
    if (!handle.isValid())
    {
        qWarning() << "Call to interrupt() on db object, but driver handle is invalid QVariant.";
        return;
    }

    interruptExecutionOnHandle(handle);
}

bool DbQt::isOpenInternal()
{
    if (!db)
        return false;

    return db->isValid() && db->isOpen();
}

bool DbQt::init()
{
    if (QSqlDatabase::database(name, false).isValid())
    {
        qCritical() << "Database with name " << name << " already exists in Qt database connections.";
        return false;
    }

    // The db is a pointer, because having an object (and not the pointer) in class members
    // causes QSqlDatabase to print warning when removing database (in constructor) that still
    // has an instance in class members.
    db = new QSqlDatabase(QSqlDatabase::addDatabase(driverName, name));
    db->setDatabaseName(path);
    db->setConnectOptions(connOptions.value("options", "").toString());

    db->open();
    QVariant value = exec("SELECT sqlite_version()")->getSingleCell();
    db->close();

    version = value.toString().mid(0, 1).toUInt();

    // TODO: register collations, custom functions, etc

    return true;
}

SqlResultsPtr DbQt::execInternal(const QString &query, const QList<QVariant> &args)
{
    if (!db)
        return SqlResultsPtr(new SqlResultsQt());

    QSqlQuery dbQuery(*db);
    if (!isOpenInternal())
        return SqlResultsPtr(new SqlResultsQt());

    dbQuery.prepare(query);
    if (dbQuery.lastError().number() != -1)
        return SqlResultsPtr(new SqlResultsQt(dbQuery, this, args));

    QVariant arg;
    foreach (arg, args)
        dbQuery.addBindValue(arg);

    dbQuery.exec();
    if (dbQuery.lastError().number() != -1)
        return SqlResultsPtr(new SqlResultsQt(dbQuery, this, args));

    return SqlResultsPtr(new SqlResultsQt(dbQuery, this, args));
}

SqlResultsPtr DbQt::execInternal(const QString &query, const QHash<QString, QVariant>& args)
{
    if (!db)
        return SqlResultsPtr(new SqlResultsQt());

    QSqlQuery dbQuery(*db);
    if (!isOpenInternal())
        return SqlResultsPtr(new SqlResultsQt());

    dbQuery.prepare(query);
    if (dbQuery.lastError().number() != -1)
        return SqlResultsPtr(new SqlResultsQt(dbQuery, this, args));

    QHashIterator<QString, QVariant> i(args);
    while (i.hasNext())
    {
        i.next();
        dbQuery.bindValue(i.key(), i.value());
    }
    dbQuery.exec();
    if (dbQuery.lastError().number() != -1)
        return SqlResultsPtr(new SqlResultsQt(dbQuery, this, args));

    return SqlResultsPtr(new SqlResultsQt(dbQuery, this, args));
}

QString DbQt::getTypeLabel()
{
    return type;
}

QHash<QString, QVariant> DbQt::getAggregateContext(void* memPtr)
{
    if (!memPtr)
    {
        qCritical() << "Could not allocate aggregate context.";
        return QHash<QString, QVariant>();
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    if (!*aggCtxPtr)
        *aggCtxPtr = new QHash<QString,QVariant>();

    return **aggCtxPtr;
}

void DbQt::setAggregateContext(void* memPtr, const QHash<QString, QVariant>& aggregateContext)
{
    if (!memPtr)
    {
        qCritical() << "Could not extract aggregate context.";
        return;
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    **aggCtxPtr = aggregateContext;
}

void DbQt::releaseAggregateContext(void* memPtr)
{
    if (!memPtr)
    {
        qCritical() << "Could not release aggregate context.";
        return;
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    delete *aggCtxPtr;
}

QVariant DbQt::evaluateScalar(void* dataPtr, const QList<QVariant>& argList, bool& ok)
{
    if (!dataPtr)
        return QVariant();

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);

    return FUNCTIONS->evaluateScalar(userData->name, userData->argCount, argList, userData->db, ok);
}

void DbQt::evaluateAggregateStep(void* dataPtr, QHash<QString,QVariant>& aggregateContext, QList<QVariant> argList)
{
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);

    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();
    if (!aggregateContext.contains("initExecuted"))
    {
        FUNCTIONS->evaluateAggregateInitial(userData->name, userData->argCount, userData->db, storage);
        aggregateContext["initExecuted"] = true;
    }

    FUNCTIONS->evaluateAggregateStep(userData->name, userData->argCount, argList, userData->db, storage);
    aggregateContext["storage"] = storage;
}

QVariant DbQt::evaluateAggregateFinal(void* dataPtr, QHash<QString,QVariant>& aggregateContext, bool& ok)
{
    if (!dataPtr)
        return QVariant();

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();

    return FUNCTIONS->evaluateAggregateFinal(userData->name, userData->argCount, userData->db, ok, storage);
}
