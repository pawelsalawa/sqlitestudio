#include "dbpluginqt.h"
#include "dbqt.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QFileInfo>

DbPluginQt::DbPluginQt()
{
}

DbPluginQt::~DbPluginQt()
{
    if (!probeConnName.isNull())
        QSqlDatabase::removeDatabase(probeConnName);
}

Db* DbPluginQt::getInstance(const QString &path, const QHash<QString,QVariant> &options)
{
    if (!probe(path, options))
        return nullptr;

    return getInstance();
}

QString DbPluginQt::generateDbName(const QVariant &baseValue)
{
    QFileInfo file(baseValue.toString());
    return file.baseName();
}

QList<DbPluginOption> DbPluginQt::getOptionsList() const
{
    return QList<DbPluginOption>();
}

bool DbPluginQt::isRemote() const
{
    return false;
}

bool DbPluginQt::init()
{
    return true;
}

void DbPluginQt::deinit()
{
}

bool DbPluginQt::probe(const QString &path, const QHash<QString,QVariant> &options)
{
    if (probeConnName.isNull())
        initProbeConnection();

    QSqlDatabase probeDb = QSqlDatabase::database(probeConnName, false);
    probeDb.setDatabaseName(path);
    probeDb.setConnectOptions(options.value("options", "").toString());
    if (!probeDb.open())
        return false;

    probeDb.exec("SELECT * FROM sqlite_master");

    bool ok = false;
    if (probeDb.lastError().type() == QSqlError::NoError)
        ok = true;

    probeDb.close();
    return ok;
}

void DbPluginQt::initProbeConnection()
{
    QString type = getDriver();
    probeConnName = "probe_connection_"+type;
    QSqlDatabase::addDatabase(type, probeConnName);
}
