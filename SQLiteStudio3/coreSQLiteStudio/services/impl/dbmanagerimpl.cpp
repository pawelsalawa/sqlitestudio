#include "dbmanagerimpl.h"
#include "db/db.h"
#include "services/config.h"
#include "plugins//dbplugin.h"
#include "services/pluginmanager.h"
#include "services/notifymanager.h"
#include "common/utils.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QHashIterator>
#include <QPluginLoader>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <db/invaliddb.h>

DbManagerImpl::DbManagerImpl(QObject *parent) :
    DbManager(parent)
{
    init();
}

DbManagerImpl::~DbManagerImpl()
{
//    qDebug() << "DbManagerImpl::~DbManagerImpl()";
    for (Db*& db : dbList)
    {
        disconnect(db, SIGNAL(disconnected()), this, SLOT(dbDisconnectedSlot()));
        disconnect(db, SIGNAL(aboutToDisconnect(bool&)), this, SLOT(dbAboutToDisconnect(bool&)));
        if (db->isOpen())
            db->close();

        delete db;
    }
    dbList.clear();
    nameToDb.clear();
    pathToDb.clear();
}

bool DbManagerImpl::addDb(const QString &name, const QString &path, bool permanent)
{
    return addDb(name, path, QHash<QString,QVariant>(), permanent);
}

bool DbManagerImpl::addDb(const QString &name, const QString &path, const QHash<QString,QVariant>& options, bool permanent)
{
    if (getByName(name))
    {
        qWarning() << "Tried to add database with name that was already on the list:" << name;
        return false;
    }

    if (getByPath(path))
    {
        qWarning() << "Tried to add database with path that was already on the list:" << path;
        return false;
    }

    QString errorMessage;
    Db* db = createDb(name, path, options, &errorMessage);
    if (!db)
    {
        notifyError(tr("Could not add database %1: %2").arg(path, errorMessage));
        return false;
    }

    listLock.lockForWrite();
    addDbInternal(db, permanent);
    listLock.unlock();

    emit dbAdded(db);

    return true;
}

bool DbManagerImpl::updateDb(Db* db, const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent)
{
    if (db->isOpen())
    {
        if (!db->close())
            return false;
    }

    QString normalizedPath;
    QUrl url(path);
    if (url.scheme().isEmpty() || url.scheme() == "file")
        normalizedPath = QDir(path).absolutePath();
    else
        normalizedPath = path;

    listLock.lockForWrite();
    nameToDb.remove(db->getName(), Qt::CaseInsensitive);
    pathToDb.remove(db->getPath());

    QString oldName = db->getName();
    db->setName(name);
    db->setPath(normalizedPath);
    db->setConnectionOptions(options);

    bool result = true;
    if (permanent)
    {
        if (CFG->isDbInConfig(oldName))
            result = CFG->updateDb(oldName, name, normalizedPath, options);
        else
            result = CFG->addDb(name, normalizedPath, options);
    }
    else if (CFG->isDbInConfig(name)) // switched "permanent" off?
        result = CFG->removeDb(name);

    InvalidDb* invalidDb = dynamic_cast<InvalidDb*>(db);
    bool wasReloaded = false;
    Db* reloadedDb = db;
    if (invalidDb)
    {
        reloadedDb = tryToLoadDb(invalidDb, false);
        if (reloadedDb) // we need to know that, so we can emit dbLoaded() signal later, out of the listLock
            wasReloaded = true;
    }

    if (reloadedDb) // reloading was not necessary (was not invalid) or it was successful
        db = reloadedDb;

    nameToDb[name] = db;
    pathToDb[normalizedPath] = db;

    listLock.unlock();

    // If we did reload the db, we need to emit proper signal, because it was suppressed in tryToLoadDb(), because of the listLock
    if (wasReloaded)
        emit dbLoaded(db);

    if (result && reloadedDb)
        emit dbUpdated(oldName, db);
    else if (reloadedDb) // database reloaded correctly, but update failed
        notifyError(tr("Database %1 could not be updated, because of an error: %2").arg(oldName, CFG->getLastErrorString()));

    return result;
}

void DbManagerImpl::removeDbByName(const QString &name, Qt::CaseSensitivity cs)
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

    delete db;
}

void DbManagerImpl::removeDbByPath(const QString &path)
{
    // Using QDir to normalize separator
    QDir pathDir(path);

    listLock.lockForRead();
    bool contains = pathToDb.contains(pathDir.absolutePath());
    listLock.unlock();
    if (!contains)
        return;

    listLock.lockForWrite();
    Db* db = pathToDb[pathDir.absolutePath()];
    removeDbInternal(db);
    listLock.unlock();

    emit dbRemoved(db);

    delete db;
}

void DbManagerImpl::removeDb(Db* db)
{
    db->close();

    listLock.lockForWrite();
    removeDbInternal(db);
    listLock.unlock();

    emit dbRemoved(db);
    delete db;
}

void DbManagerImpl::removeDbInternal(Db* db, bool alsoFromConfig)
{
    QString name = db->getName();
    if (alsoFromConfig)
        CFG->removeDb(name);

    nameToDb.remove(name);
    pathToDb.remove(db->getPath());
    dbList.removeOne(db);
    disconnect(db, SIGNAL(connected()), this, SLOT(dbConnectedSlot()));
    disconnect(db, SIGNAL(disconnected()), this, SLOT(dbDisconnectedSlot()));
    disconnect(db, SIGNAL(aboutToDisconnect(bool&)), this, SLOT(dbAboutToDisconnect(bool&)));
}

QList<Db*> DbManagerImpl::getDbList()
{
    listLock.lockForRead();
    QList<Db*> list = dbList;
    listLock.unlock();
    return list;
}

QList<Db*> DbManagerImpl::getValidDbList()
{
    QList<Db*> list = getDbList();
    QMutableListIterator<Db*> it(list);
    while (it.hasNext())
    {
        it.next();
        if (!it.value()->isValid())
            it.remove();
    }

    return list;
}

QList<Db*> DbManagerImpl::getConnectedDbList()
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

QStringList DbManagerImpl::getDbNames()
{
    QReadLocker lock(&listLock);
    return nameToDb.keys();
}

QStringList DbManagerImpl::getValidDbNames()
{
    QReadLocker lock(&listLock);
    QStringList result;
    for (Db* db : dbList)
    {
        if (db->isValid())
            result << db->getName();
    }
    return result;
}

Db* DbManagerImpl::getByName(const QString &name, Qt::CaseSensitivity cs)
{
    QReadLocker lock(&listLock);
    return nameToDb.value(name, cs);
}

Db* DbManagerImpl::getByPath(const QString &path)
{
    // Using QDir to normalize separator
    QDir pathDir(path);
    return pathToDb.value(pathDir.absolutePath());
}

Db* DbManagerImpl::createInMemDb(bool pureInit)
{
    if (!inMemDbCreatorPlugin)
        return nullptr;

    QHash<QString, QVariant> opts;
    if (pureInit)
        opts[DB_PURE_INIT] = true;

    return inMemDbCreatorPlugin->getInstance("", ":memory:", opts);
}

bool DbManagerImpl::isTemporary(Db* db)
{
    return CFG->getDb(db->getName()).isNull();
}

QString DbManagerImpl::quickAddDb(const QString& path, const QHash<QString, QVariant>& options)
{
    QString newName = DbManager::generateDbName(path);
    newName = generateUniqueName(newName, DBLIST->getDbNames());
    if (!DBLIST->addDb(newName, path, options, false))
        return QString();

    return newName;
}

DbPlugin* DbManagerImpl::getPluginForDbFile(const QString& filePath)
{
    QFileInfo file(filePath);
    if (!file.exists() || file.isDir())
        return nullptr;

    QHash<QString,QVariant> options;
    Db* probeDb = nullptr;
    for (DbPlugin*& plugin : dbPlugins)
    {
        probeDb = plugin->getInstance("", filePath, options);
        if (probeDb)
        {
            delete probeDb;
            probeDb = nullptr;
            return plugin;
        }
    }

    return nullptr;
}

QString DbManagerImpl::generateUniqueDbName(const QString& filePath)
{
    DbPlugin* plugin = getPluginForDbFile(filePath);
    if (!plugin)
        return QString();

    return generateUniqueDbName(plugin, filePath);
}

QString DbManagerImpl::generateUniqueDbName(DbPlugin* plugin, const QString& filePath)
{
    QString name = plugin->generateDbName(filePath);
    name = generateUniqueName(name, getDbNames(), Qt::CaseInsensitive);
    return name;
}

void DbManagerImpl::setInMemDbCreatorPlugin(DbPlugin* plugin)
{
    inMemDbCreatorPlugin = plugin;
}

void DbManagerImpl::init()
{
    Q_ASSERT(PLUGINS);

    loadInitialDbList();

    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(aboutToUnload(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(loaded(Plugin*,PluginType*)));
}

void DbManagerImpl::loadInitialDbList()
{
    QUrl url;
    InvalidDb* db = nullptr;
    for (Config::CfgDbPtr& cfgDb : CFG->dbList())
    {
        db = new InvalidDb(cfgDb->name, cfgDb->path, cfgDb->options);

        url = QUrl::fromUserInput(cfgDb->path);
        if (url.isLocalFile() && !QFile::exists(cfgDb->path))
            db->setError(tr("Database file doesn't exist."));
        else
            db->setError(tr("No supporting plugin loaded."));

        addDbInternal(db, false);
    }
}

void DbManagerImpl::notifyDatabasesAreLoaded()
{
    // Any databases were already loaded by loaded() slot, which is called when DbPlugin was loaded.
    emit dbListLoaded();
}

void DbManagerImpl::scanForNewDatabasesInConfig()
{
    QList<Config::CfgDbPtr> cfgDbList = CFG->dbList();

    QUrl url;
    InvalidDb* db = nullptr;
    for (Config::CfgDbPtr& cfgDb : cfgDbList)
    {
        if (getByName(cfgDb->name) || getByPath(cfgDb->path))
            continue;

        db = new InvalidDb(cfgDb->name, cfgDb->path, cfgDb->options);

        url = QUrl::fromUserInput(cfgDb->path);
        if (url.isLocalFile() && !QFile::exists(cfgDb->path))
            db->setError(tr("Database file doesn't exist."));
        else
            db->setError(tr("No supporting plugin loaded."));

        addDbInternal(db);
        tryToLoadDb(db);
    }
}

void DbManagerImpl::rescanInvalidDatabasesForPlugin(DbPlugin* dbPlugin)
{
    if (!dbPlugin)
    {
        qWarning() << "Call to DbManagerImpl::rescanInvalidDatabasesForPlugin() with null plugin.";
        return;
    }

    Db* db = nullptr;

    QUrl url;
    QString errorMessages;
    for (Db*& invalidDb : getInvalidDatabases())
    {
        if (invalidDb->getConnectionOptions().contains(DB_PLUGIN) && invalidDb->getConnectionOptions()[DB_PLUGIN].toString() != dbPlugin->getName())
            continue;

        url = QUrl::fromUserInput(invalidDb->getPath());
        if (url.isLocalFile() && !QFile::exists(invalidDb->getPath()) && invalidDb->getPath() != ":memory:")
            continue;

        errorMessages = QString();
        db = createDb(invalidDb->getName(), invalidDb->getPath(), invalidDb->getConnectionOptions(), &errorMessages);
        if (!db)
        {
            if (!errorMessages.isNull())
            {
                dynamic_cast<InvalidDb*>(invalidDb)->setError(errorMessages);
            }
            continue; // For this db driver was not loaded yet.
        }

        if (!dbPlugin->checkIfDbServedByPlugin(db))
        {
            qDebug().noquote() << "Managed to load database" << toNativePath(db->getPath()) << " (" << db->getName() << ")"
                     << "but it doesn't use DbPlugin that was just loaded, so it will not be loaded to the db manager";

            delete db;
            continue;
        }

        removeDbInternal(invalidDb, false);
        delete invalidDb;

        addDbInternal(db, false);

        if (!db->getConnectionOptions().contains(DB_PLUGIN))
        {
            db->getConnectionOptions()[DB_PLUGIN] = dbPlugin->getName();
            if (!CFG->updateDb(db->getName(), db->getName(), db->getPath(), db->getConnectionOptions()))
                qWarning() << "Could not store handling plugin in options for database" << db->getName();
        }

        if (CFG->getDbGroup(db->getName())->open)
            db->open();

        emit dbLoaded(db);
    }
}

void DbManagerImpl::addDbInternal(Db* db, bool alsoToConfig)
{
    if (alsoToConfig)
        CFG->addDb(db->getName(), db->getPath(), db->getConnectionOptions());

    dbList << db;
    nameToDb[db->getName()] = db;
    pathToDb[db->getPath()] = db;
    connect(db, SIGNAL(connected()), this, SLOT(dbConnectedSlot()));
    connect(db, SIGNAL(disconnected()), this, SLOT(dbDisconnectedSlot()));
    connect(db, SIGNAL(aboutToDisconnect(bool&)), this, SLOT(dbAboutToDisconnect(bool&)));
}

QList<Db*> DbManagerImpl::getInvalidDatabases() const
{
    return filter<Db*>(dbList, [](Db* db) -> bool
    {
        return !db->isValid();
    });
}

Db* DbManagerImpl::tryToLoadDb(InvalidDb* invalidDb, bool emitNotifySignal)
{
    Db* db = createDb(invalidDb->getName(), invalidDb->getPath(), invalidDb->getConnectionOptions());
    if (!db)
        return nullptr;

    removeDbInternal(invalidDb, false);
    delete invalidDb;

    addDbInternal(db, false);

    if (CFG->getDbGroup(db->getName())->open)
        db->open();

    if (emitNotifySignal)
        emit dbLoaded(db);

    return db;
}

Db* DbManagerImpl::createDb(const QString &name, const QString &path, const QHash<QString,QVariant> &options, QString* errorMessages)
{
    QList<DbPlugin*> dbPlugins = PLUGINS->getLoadedPlugins<DbPlugin>();
    Db* db = nullptr;
    QStringList messages;
    QString message;

    QString normalizedPath;
    QUrl url(path);
    if (url.scheme().isEmpty() || url.scheme() == "file")
        normalizedPath = QDir(path).absolutePath();
    else
        normalizedPath = path;

    for (DbPlugin* dbPlugin : dbPlugins)
    {
        if (options.contains("plugin") && options["plugin"] != dbPlugin->getName())
            continue;

        db = dbPlugin->getInstance(name, normalizedPath, options, &message);
        if (!db)
        {
            messages << message;
            continue;
        }

        if (!db->initAfterCreated())
        {
            safe_delete(db);
            messages << tr("Database could not be initialized.");
            continue;
        }

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


void DbManagerImpl::dbConnectedSlot()
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

void DbManagerImpl::dbDisconnectedSlot()
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

void DbManagerImpl::dbAboutToDisconnect(bool& deny)
{
    QObject* sdr = sender();
    Db* db = dynamic_cast<Db*>(sdr);
    if (!db)
    {
        qWarning() << "Received dbAboutToDisconnect() signal but could not cast it to Db!";
        return;
    }
    emit dbAboutToBeDisconnected(db, deny);
}

void DbManagerImpl::aboutToUnload(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<DbPlugin>())
        return;

    InvalidDb* invalidDb = nullptr;
    DbPlugin* dbPlugin = dynamic_cast<DbPlugin*>(plugin);
    dbPlugins.removeOne(dbPlugin);
    QList<Db*> toRemove;
    for (Db*& db : dbList)
    {
        if (!dbPlugin->checkIfDbServedByPlugin(db))
            continue;

        toRemove << db;
    }

    for (Db*& db : toRemove)
    {
        emit dbAboutToBeUnloaded(db, dbPlugin);

        if (db->isOpen())
            db->close();

        removeDbInternal(db, false);

        invalidDb = new InvalidDb(db->getName(), db->getPath(), db->getConnectionOptions());
        invalidDb->setError(tr("No supporting plugin loaded."));
        addDbInternal(invalidDb, false);

        delete db;

        emit dbUnloaded(invalidDb);
    }
}

void DbManagerImpl::loaded(Plugin* plugin, PluginType* type)
{
    if (!type->isForPluginType<DbPlugin>())
        return;

    DbPlugin* dbPlugin = dynamic_cast<DbPlugin*>(plugin);
    dbPlugins << dbPlugin;
    rescanInvalidDatabasesForPlugin(dbPlugin);
}
