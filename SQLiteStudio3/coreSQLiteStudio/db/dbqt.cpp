#include "dbqt.h"
#include "sqlitestudio.h"
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
