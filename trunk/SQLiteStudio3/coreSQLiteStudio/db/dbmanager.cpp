#include "dbmanager.h"
#include "db.h"
#include "sqlitestudio.h"
#include "config.h"
#include "dbplugin.h"
#include "pluginmanager.h"
#include "notifymanager.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QHashIterator>
#include <QPluginLoader>
#include <QDebug>

DbManager::DbManager(QObject *parent) :
    QObject(parent)
{
    init();
}

bool DbManager::addDb(const QString &name, const QString &path, bool permanent)
{
    return addDb(name, path, QHash<QString,QVariant>(), permanent);
}

bool DbManager::addDb(const QString &name, const QString &path, const QHash<QString,QVariant>& options, bool permanent)
{
    QString errorMessage;
    Db* db = createDbObj(name, path, options, &errorMessage);
    if (!db)
    {
        notifyError(tr("Could not add database %1: %2").arg(path).arg(errorMessage));
        return false;
    }

    listLock.lockForWrite();
    addDbInternal(db, permanent);
    listLock.unlock();

    emit dbAdded(db);

    return true;
}

bool DbManager::updateDb(Db* db, const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent)
{
    if (db->isOpen())
    {
        if (!db->close())
            return false;
    }

    listLock.lockForWrite();
    nameToDb.remove(db->getName(), Qt::CaseInsensitive);
    pathToDb.remove(db->getPath());

    QString oldName = db->name;
    db->name = name;
    db->path = path;
    db->connOptions = options;

    bool result = false;
    if (permanent)
    {
        if (CFG->isDbInConfig(oldName))
            result = CFG->updateDb(oldName, name, path, options);
        else
            result = CFG->addDb(name, path, options);
    }
    else if (CFG->isDbInConfig(name)) // switched "permanent" off?
        result = CFG->removeDb(name);

    nameToDb[name] = db;
    pathToDb[path] = db;

    listLock.unlock();

    if (result)
        emit dbUpdated(oldName, db);
    else
        notifyError(tr("Database %1 could not be updated, because of an error: %2").arg(oldName).arg(CFG->getLastErrorString()));

    return result;
}

void DbManager::removeDbByName(const QString &name, Qt::CaseSensitivity cs)
{
    listLock.lockForRead();
    bool contains = nameToDb.contains(name, cs);
    listLock.unlock();

    if (!contains)
        return;

    listLock.lockForWrite();
    Db* db = nameToDb[name];
    removeDbInternal(db);
    listLock.unlock();

    emit dbRemoved(db);
}

void DbManager::removeDbByPath(const QString &path)
{
    listLock.lockForRead();
    bool contains = pathToDb.contains(path);
    listLock.unlock();
    if (!contains)
        return;

    listLock.lockForWrite();
    Db* db = pathToDb[path];
    removeDbInternal(db);
    listLock.unlock();

    emit dbRemoved(db);
}

void DbManager::removeDb(Db* db)
{
    listLock.lockForWrite();
    removeDbInternal(db);
    listLock.unlock();

    emit dbRemoved(db);
}

void DbManager::removeDbInternal(Db* db, bool alsoFromConfig)
{
    QString name = db->getName();
    if (alsoFromConfig)
        CFG->removeDb(name);

    nameToDb.remove(name);
    pathToDb.remove(db->getPath());
    dbList.removeOne(db);
    disconnect(db, &Db::connected, this, &DbManager::dbConnectedSlot);
    disconnect(db, &Db::disconnected, this, &DbManager::dbDisconnectedSlot);
}

QList<Db*> DbManager::getDbList()
{
    listLock.lockForRead();
    QList<Db*> list = dbList;
    listLock.unlock();
    return list;
}

QList<Db*> DbManager::getConnectedDbList()
{
    QList<Db*> list = getDbList();
    QMutableListIterator<Db*> it(list);
    while (it.hasNext())
    {
        it.next();
        if (!it.value()->isOpen())
            it.remove();
    }

    return list;
}

QStringList DbManager::getDbNames()
{
    QReadLocker lock(&listLock);
    return nameToDb.keys();
}

Db* DbManager::getByName(const QString &name, Qt::CaseSensitivity cs)
{
    QReadLocker lock(&listLock);
    return nameToDb.value(name, cs);
}

Db* DbManager::getByPath(const QString &path)
{
    return pathToDb.value(path);
}

QString DbManager::generateDbName(const QString &filePath)
{
    QFileInfo fi(filePath);
    return fi.completeBaseName();
}

void DbManager::init()
{
    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(aboutToUnload(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(loaded(Plugin*,PluginType*)));
}

void DbManager::loadDbListFromConfig()
{
    // Any databases were already loaded by loaded() slot, which is called when DbPlugin was loaded.
    emit dbListLoaded();
}

void DbManager::addDbInternal(Db* db, bool alsoToConfig)
{
    if (alsoToConfig)
        CFG->addDb(db->getName(), db->getPath(), db->getConnectionOptions());

    dbList << db;
    nameToDb[db->getName()] = db;
    pathToDb[db->getPath()] = db;
    connect(db, &Db::connected, this, &DbManager::dbConnectedSlot);
    connect(db, &Db::disconnected, this, &DbManager::dbDisconnectedSlot);
}

Db* DbManager::createDbObj(const QString &name, const QString &path, const QHash<QString,QVariant> &options, QString* errorMessages)
{
    QList<DbPlugin*> dbPlugins = PLUGINS->getLoadedPlugins<DbPlugin>();
    DbPlugin* dbPlugin;
    Db* db;
    QStringList messages;
    QString message;
    foreach (dbPlugin, dbPlugins)
    {
        if (options.contains("plugin") && options["plugin"] != dbPlugin->getName())
            continue;

        db = dbPlugin->getInstance(path, options, &message);
        if (!db)
        {
            messages << message;
            continue;
        }

        db->init(name, path, options);
        return db;
    }

    if (errorMessages)
    {
        if (messages.size() == 0)
            messages << tr("No suitable database driver plugin found.");

        *errorMessages = messages.join("; ");
    }

    return nullptr;
}


void DbManager::dbConnectedSlot()
{
    QObject* sdr = sender();
    Db* db = dynamic_cast<Db*>(sdr);
    if (!db)
    {
        qWarning() << "Received connected() signal but could not cast it to Db!";
        return;
    }
    emit dbConnected(db);
}

void DbManager::dbDisconnectedSlot()
{
    QObject* sdr = sender();
    Db* db = dynamic_cast<Db*>(sdr);
    if (!db)
    {
        qWarning() << "Received disconnected() signal but could not cast it to Db!";
        return;
    }
    emit dbDisconnected(db);
}

void DbManager::aboutToUnload(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<DbPlugin>())
        return;

    DbPlugin* dbPlugin = dynamic_cast<DbPlugin*>(plugin);
    foreach (Db* db, dbList)
    {
        if (!dbPlugin->checkIfDbServedByPlugin(db))
            continue;

        emit dbAboutToBeUnloaded(db, dbPlugin);

        if (db->isOpen())
            db->close();

        removeDbInternal(db, false);
        delete db;
    }
}

void DbManager::loaded(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<DbPlugin>())
        return;

    DbPlugin* dbPlugin = dynamic_cast<DbPlugin*>(plugin);
    Db* db = nullptr;
    foreach (const Config::CfgDbPtr& cfgDb, CFG->dbList())
    {
        if (getByName(cfgDb->name))
            continue;

        db = createDbObj(cfgDb->name, cfgDb->path, cfgDb->options);
        if (!db)
        {
            // For this db driver was not loaded yet.
            continue;
        }

        if (!dbPlugin->checkIfDbServedByPlugin(db))
        {
            qDebug() << "Managed to load database" << db->getPath() << " (" << db->getName() << ")"
                     << "but it doesn't use DbPlugin that was just loaded, so it will not be loaded to the db manager";

            delete db;
            continue;
        }

        addDbInternal(db, false);

        if (CFG->getDbGroup(cfgDb->name)->open)
            db->open();

        emit dbLoaded(db, dbPlugin);
    }
}
