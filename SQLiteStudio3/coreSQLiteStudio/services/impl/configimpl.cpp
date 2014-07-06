#include "configimpl.h"
#include "sqlhistorymodel.h"
#include "ddlhistorymodel.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
#include "db/dbsqlite3.h"
#include <QtGlobal>
#include <QDebug>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QDataStream>
#include <QRegExp>
#include <QDateTime>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrentRun>

static_qstring(DB_FILE_NAME, "settings3");

ConfigImpl::~ConfigImpl()
{
    cleanUp();
}

void ConfigImpl::init()
{
    initDbFile();
    initTables();

    connect(this, SIGNAL(sqlHistoryRefreshNeeded()), this, SLOT(refreshSqlHistory()));
}

void ConfigImpl::cleanUp()
{
    if (db->isOpen())
        db->close();

    safe_delete(db);
}

const QString &ConfigImpl::getConfigDir()
{
    return configDir;
}

void ConfigImpl::beginMassSave()
{
    emit massSaveBegins();
    db->exec("BEGIN;");
}

void ConfigImpl::commitMassSave()
{
    db->exec("COMMIT;");
    emit massSaveCommited();
}

void ConfigImpl::rollbackMassSave()
{
    db->exec("ROLLBACK;");
}

void ConfigImpl::set(const QString &group, const QString &key, const QVariant &value)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << value;

    db->exec("INSERT OR REPLACE INTO settings VALUES (?, ?, ?)", {group, key, bytes});
}

QVariant ConfigImpl::get(const QString &group, const QString &key)
{
    SqlQueryPtr results = db->exec("SELECT value FROM settings WHERE [group] = ? AND [key] = ?", {group, key});
    return deserializeValue(results->getSingleCell());
}

QHash<QString,QVariant> ConfigImpl::getAll()
{
    SqlQueryPtr results = db->exec("SELECT [group], [key], value FROM settings");

    QHash<QString,QVariant> cfg;
    QString key;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        key = row->value("group").toString() + "." + row->value("key").toString();
        cfg[key] = deserializeValue(row->value("value"));
    }
    return cfg;
}

bool ConfigImpl::storeErrorAndReturn(SqlQueryPtr results)
{
    if (results->isError())
    {
        lastQueryError = results->getErrorText();
        return true;
    }
    else
        return false;
}

void ConfigImpl::printErrorIfSet(SqlQueryPtr results)
{
    if (results && results->isError())
    {
        qCritical() << "Config error while executing query:" << results->getErrorText();
        storeErrorAndReturn(results);
    }
}

bool ConfigImpl::addDb(const QString& name, const QString& path, const QHash<QString,QVariant>& options)
{
    SqlQueryPtr results = db->exec("INSERT INTO dblist VALUES (?, ?, ?)", {name, path, options});
    return !storeErrorAndReturn(results);
}

bool ConfigImpl::updateDb(const QString &name, const QString &newName, const QString &path, const QHash<QString,QVariant> &options)
{
    SqlQueryPtr results = db->exec("UPDATE dblist SET name = ?, path = ?, options = ? WHERE name = ?",
                                     {name, newName, path, options});

    return (!storeErrorAndReturn(results)  && results->rowsAffected() > 0);
}

bool ConfigImpl::removeDb(const QString &name)
{
    SqlQueryPtr results = db->exec("DELETE FROM dblist WHERE name = ?", {name});
    return (!storeErrorAndReturn(results) && results->rowsAffected() > 0);
}

bool ConfigImpl::isDbInConfig(const QString &name)
{
    SqlQueryPtr results = db->exec("SELECT * FROM dblist WHERE name = ?", {name});
    return (!storeErrorAndReturn(results) && results->hasNext());
}

QString ConfigImpl::getLastErrorString() const
{
    QString msg = db->getErrorText();
    if (msg.trimmed().isEmpty())
        return lastQueryError;

    return msg;
}

QList<ConfigImpl::CfgDbPtr> ConfigImpl::dbList()
{
    QList<CfgDbPtr> entries;
    SqlQueryPtr results = db->exec("SELECT name, path, options FROM dblist");
    CfgDbPtr cfgDb;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        cfgDb = CfgDbPtr::create();
        cfgDb->name = row->value("name").toString();
        cfgDb->path = row->value("path").toString();
        cfgDb->options = row->value("options").toHash();
        entries += cfgDb;
    }

    return entries;
}

ConfigImpl::CfgDbPtr ConfigImpl::getDb(const QString& dbName)
{
    SqlQueryPtr results = db->exec("SELECT path, options FROM dblist WHERE name = ?", {dbName});

    if (!results->hasNext())
        return CfgDbPtr();

    SqlResultsRowPtr row = results->next();

    CfgDbPtr cfgDb = CfgDbPtr::create();
    cfgDb->name = dbName;
    cfgDb->path = row->value("path").toString();
    cfgDb->options = row->value("options").toHash();
    return cfgDb;
}

void ConfigImpl::storeGroups(const QList<DbGroupPtr>& groups)
{
    db->exec("DELETE FROM groups");

    foreach (const DbGroupPtr& group, groups)
        storeGroup(group);
}

void ConfigImpl::storeGroup(const ConfigImpl::DbGroupPtr &group, qint64 parentId)
{
    QVariant parent = QVariant(QVariant::LongLong);
    if (parentId > -1)
        parent = parentId;

    SqlQueryPtr results = db->exec("INSERT INTO groups (name, [order], parent, open, dbname) VALUES (?, ?, ?, ?, ?)",
                                    {group->name, group->order, parent, group->open, group->referencedDbName});

    qint64 newParentId = results->getRegularInsertRowId();
    foreach (const DbGroupPtr& childGroup, group->childs)
        storeGroup(childGroup, newParentId);
}

QList<ConfigImpl::DbGroupPtr> ConfigImpl::getGroups()
{
    DbGroupPtr topGroup = DbGroupPtr::create();
    topGroup->id = -1;
    readGroupRecursively(topGroup);
    return topGroup->childs;
}

ConfigImpl::DbGroupPtr ConfigImpl::getDbGroup(const QString& dbName)
{
    SqlQueryPtr results = db->exec("SELECT id, name, [order], open, dbname FROM groups WHERE dbname = ? LIMIT 1", {dbName});

    DbGroupPtr group = DbGroupPtr::create();
    group->referencedDbName = dbName;

    if (!results->hasNext())
        return group;

    SqlResultsRowPtr row = results->next();
    group->id = row->value("id").toULongLong();
    group->name = row->value("name").toString();
    group->order = row->value("order").toInt();
    group->open = row->value("open").toBool();
    return group;
}

qint64 ConfigImpl::addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    SqlQueryPtr results = db->exec("INSERT INTO sqleditor_history (dbname, date, time_spent, rows, sql) VALUES (?, ?, ?, ?, ?)",
                                    {dbName, (QDateTime::currentMSecsSinceEpoch() / 1000), timeSpentMillis, rowsAffected, sql});

    qint64 rowId = results->getRegularInsertRowId();
    QtConcurrent::run(this, &ConfigImpl::asyncApplySqlHistoryLimit);
    return rowId;
}

void ConfigImpl::updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    QtConcurrent::run(this, &ConfigImpl::asyncUpdateSqlHistory, id, sql, dbName, timeSpentMillis, rowsAffected);
}

void ConfigImpl::clearSqlHistory()
{
    QtConcurrent::run(this, &ConfigImpl::asyncClearSqlHistory);
}

QAbstractItemModel* ConfigImpl::getSqlHistoryModel()
{
    if (!sqlHistoryModel)
        sqlHistoryModel = new SqlHistoryModel(db, this);

    return sqlHistoryModel;
}

void ConfigImpl::addCliHistory(const QString& text)
{
    QtConcurrent::run(this, &ConfigImpl::asyncAddCliHistory, text);
}

void ConfigImpl::applyCliHistoryLimit()
{
    QtConcurrent::run(this, &ConfigImpl::asyncApplyCliHistoryLimit);
}

void ConfigImpl::clearCliHistory()
{
    static_qstring(clearQuery, "DELETE FROM cli_history");

    SqlQueryPtr results = db->exec(clearQuery);
    if (results->isError())
        qWarning() << "Error while clearing CLI history:" << db->getErrorText();
}

QStringList ConfigImpl::getCliHistory() const
{
    static_qstring(selectQuery, "SELECT text FROM cli_history ORDER BY id");

    SqlQueryPtr results = db->exec(selectQuery);
    if (results->isError())
        qWarning() << "Error while getting CLI history:" << db->getErrorText();

    return results->columnAsList<QString>("text");
}

void ConfigImpl::addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile)
{
    db->exec("INSERT INTO ddl_history (dbname, file, timestamp, queries) VALUES (?, ?, ?, ?)",
                {dbName, dbFile, QDateTime::currentDateTime().toTime_t(), queries});

    int maxHistorySize = CFG_CORE.General.DdlHistorySize.get();

    SqlQueryPtr results = db->exec("SELECT count(*) FROM ddl_history");
    if (results->hasNext() && results->getSingleCell().toInt() > maxHistorySize)
    {
        results = db->exec(QString("SELECT id FROM ddl_history ORDER BY id DESC LIMIT 1 OFFSET %1").arg(maxHistorySize), Db::Flag::NO_LOCK);
        if (results->hasNext())
        {
            int id = results->getSingleCell().toInt();
            if (id > 0) // it will be 0 on fail conversion, but we won't delete id <= 0 ever.
                db->exec("DELETE FROM ddl_history WHERE id <= ?", {id});
        }
    }

    if (ddlHistoryModel)
            dynamic_cast<DdlHistoryModel*>(ddlHistoryModel)->refresh();
}

QList<ConfigImpl::DdlHistoryEntryPtr> ConfigImpl::getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date)
{
    static_qstring(sql,
            "SELECT timestamp,"
            "       queries"
            "  FROM ddl_history"
            " WHERE dbname = ?"
            "   AND file = ?"
            "   AND date(timestamp, 'unixepoch') = ?");

    SqlQueryPtr results = db->exec(sql, {dbName, dbFile, date.toString("yyyy-MM-dd")});

    QList<DdlHistoryEntryPtr> etnries;
    DdlHistoryEntryPtr entry;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        entry = DdlHistoryEntryPtr::create();
        entry->dbName = dbName;
        entry->dbFile = dbFile;
        entry->timestamp = QDateTime::fromTime_t(row->value("timestamp").toUInt());
        entry->queries = row->value("queries").toString();
        etnries << entry;
    }
    return etnries;
}

DdlHistoryModel* ConfigImpl::getDdlHistoryModel()
{
    if (!ddlHistoryModel)
        ddlHistoryModel = new DdlHistoryModel(db, this);

    return ddlHistoryModel;
}

void ConfigImpl::clearDdlHistory()
{
    db->exec("DELETE FROM ddl_history");
    if (ddlHistoryModel)
        ddlHistoryModel->refresh();
}

void ConfigImpl::readGroupRecursively(ConfigImpl::DbGroupPtr group)
{
    SqlQueryPtr results;
    if (group->id < 0)
        results = db->exec("SELECT id, name, [order], open, dbname FROM groups WHERE parent IS NULL ORDER BY [order]");
    else
        results = db->exec("SELECT id, name, [order], open, dbname FROM groups WHERE parent = ? ORDER BY [order]", {group->id});

    DbGroupPtr childGroup;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        childGroup = DbGroupPtr::create();
        childGroup->id = row->value("id").toULongLong();
        childGroup->name = row->value("name").toString();
        childGroup->order = row->value("order").toInt();
        childGroup->open = row->value("open").toBool();
        childGroup->referencedDbName = row->value("dbname").toString();
        group->childs += childGroup;
    }

    for (int i = 0; i < group->childs.size(); i++)
        readGroupRecursively(group->childs[i]);
}

void ConfigImpl::begin()
{
    db->begin();
}

void ConfigImpl::commit()
{
    db->commit();
}

void ConfigImpl::rollback()
{
    db->rollback();
}

QString ConfigImpl::getConfigPath()
{
#ifdef Q_OS_WIN
    if (QSysInfo::windowsVersion() & QSysInfo::WV_NT_based)
        return SQLITESTUDIO->getEnv("APPDATA")+"/sqlitestudio";
    else
        return SQLITESTUDIO->getEnv("HOME")+"/sqlitestudio";
#else
    return SQLITESTUDIO->getEnv("HOME")+"/.config/sqlitestudio";
#endif
}

QString ConfigImpl::getPortableConfigPath()
{
    QFileInfo file;
    QDir dir("./sqlitestudio-cfg");

    file = QFileInfo(dir.absolutePath());
    if (!file.exists())
        return dir.absolutePath();

    if (!file.isDir() || !file.isReadable() || !file.isWritable())
        return QString::null;

    foreach (file, dir.entryInfoList())
    {
        if (!file.isReadable() || !file.isWritable())
            return QString::null;
    }

    return dir.absolutePath();
}

void ConfigImpl::initTables()
{
    SqlQueryPtr results = db->exec("SELECT lower(name) AS name FROM sqlite_master WHERE type = 'table'");
    QList<QString> tables = results->columnAsList<QString>(0);

    if (!tables.contains("version"))
    {
        QString table;
        foreach (table, tables)
            db->exec("DROP TABLE "+table);

        tables.clear();
        db->exec("CREATE TABLE version (version NUMERIC)");
        db->exec("INSERT INTO version VALUES ("+QString::number(SQLITESTUDIO_CONFIG_VERSION)+")");
    }

    if (!tables.contains("settings"))
        db->exec("CREATE TABLE settings ([group] TEXT, [key] TEXT, value, PRIMARY KEY([group], [key]))");

    if (!tables.contains("sqleditor_history"))
        db->exec("CREATE TABLE sqleditor_history (id INTEGER PRIMARY KEY, dbname TEXT, date INTEGER, time_spent INTEGER, rows INTEGER, sql TEXT)");

    if (!tables.contains("dblist"))
        db->exec("CREATE TABLE dblist (name TEXT PRIMARY KEY, path TEXT UNIQUE, options TEXT)");

    if (!tables.contains("groups"))
        db->exec("CREATE TABLE groups (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, parent INTEGER REFERENCES groups(id), "
                 "[order] INTEGER, open INTEGER DEFAULT 0, dbname TEXT UNIQUE REFERENCES dblist(name) ON UPDATE CASCADE ON DELETE CASCADE, "
                 "UNIQUE(name, parent))");

    if (!tables.contains("ddl_history"))
        db->exec("CREATE TABLE ddl_history (id INTEGER PRIMARY KEY AUTOINCREMENT, dbname TEXT, file TEXT, timestamp INTEGER, "
                 "queries TEXT)");

    if (!tables.contains("cli_history"))
        db->exec("CREATE TABLE cli_history (id INTEGER PRIMARY KEY AUTOINCREMENT, text TEXT)");
}

void ConfigImpl::initDbFile()
{
    // Determinate global config location and portable one
    QString globalPath = getConfigPath();
    QString portablePath = getPortableConfigPath();

    QStringList paths;
    if (!globalPath.isNull() && !portablePath.isNull())
    {
        if (QFileInfo(portablePath).exists())
        {
            paths << portablePath+"/"+DB_FILE_NAME;
            paths << globalPath+"/"+DB_FILE_NAME;
        }
        else
        {
            paths << globalPath+"/"+DB_FILE_NAME;
            paths << portablePath+"/"+DB_FILE_NAME;
        }
    }
    else if (!globalPath.isNull())
    {
        paths << globalPath+"/"+DB_FILE_NAME;
    }
    else if (!portablePath.isNull())
    {
        paths << portablePath+"/"+DB_FILE_NAME;
    }

    // Create global config directory if not existing
    QDir dir;
    if (!globalPath.isNull())
    {
        dir = QDir(globalPath);
        if (!dir.exists())
            QDir::root().mkpath(globalPath);
    }

    // A fallback to in-memory db
    paths << ":memory:";

    // Go through all candidates and pick one
    QString path;
    foreach (path, paths)
    {
        dir = QDir(path);
        if (path != ":memory:")
            dir.cdUp();

        if (tryInitDbFile(path))
        {
            configDir = dir.absolutePath();
            break;
        }
    }

    // We ended up with in-memory one? That's not good.
    if (configDir == ":memory:")
    {
        paths.removeLast();
        notifyError(QObject::tr("Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart."
                       " Tried to initialize the file at following localizations: %1.").arg(paths.join(", ")));
    }

    db->exec("PRAGMA foreign_keys = 1;");
}

bool ConfigImpl::tryInitDbFile(const QString &dbPath)
{
    db = new DbSqlite3("SQLiteStudio settings", dbPath, {{DB_PURE_INIT, true}});
    if (!db->open())
    {
        safe_delete(db);
        return false;
    }

    SqlQueryPtr results = db->exec("SELECT * FROM sqlite_master");
    if (results->isError())
    {
        safe_delete(db);
        return false;
    }

    return true;
}

QVariant ConfigImpl::deserializeValue(const QVariant &value)
{
    if (!value.isValid())
        return QVariant();

    QByteArray bytes = value.toByteArray();
    if (bytes.isNull())
        return QVariant();

    QVariant deserializedValue;
    QDataStream stream(bytes);
    stream >> deserializedValue;
    return deserializedValue;
}

void ConfigImpl::asyncApplySqlHistoryLimit()
{
    int maxHistorySize = CFG_CORE.General.SqlHistorySize.get();

    SqlQueryPtr results = db->exec("SELECT count(*) FROM sqleditor_history");
    if (results->hasNext() && results->getSingleCell().toInt() > maxHistorySize)
    {
        results = db->exec(QString("SELECT id FROM sqleditor_history ORDER BY id DESC LIMIT 1 OFFSET %1").arg(maxHistorySize));
        if (results->hasNext())
        {
            int id = results->getSingleCell().toInt();
            if (id > 0) // it will be 0 on fail conversion, but we won't delete id <= 0 ever.
                db->exec("DELETE FROM sqleditor_history WHERE id <= ?", {id});
        }
    }

    emit sqlHistoryRefreshNeeded();
}

void ConfigImpl::asyncUpdateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    db->exec("UPDATE sqleditor_history SET dbname = ?, time_spent = ?, rows = ?, sql = ? WHERE id = ?",
            {dbName, timeSpentMillis, rowsAffected, sql, id});

    emit sqlHistoryRefreshNeeded();
}

void ConfigImpl::asyncClearSqlHistory()
{
    db->exec("DELETE FROM sqleditor_history");
    emit sqlHistoryRefreshNeeded();
}

void ConfigImpl::asyncAddCliHistory(const QString& text)
{
    static_qstring(insertQuery, "INSERT INTO cli_history (text) VALUES (?)");

    SqlQueryPtr results = db->exec(insertQuery, {text});
    if (results->isError())
        qWarning() << "Error while adding CLI history:" << results->getErrorText();

    applyCliHistoryLimit();
}

void ConfigImpl::asyncApplyCliHistoryLimit()
{
    static_qstring(limitQuery, "DELETE FROM cli_history WHERE id >= (SELECT id FROM cli_history ORDER BY id LIMIT 1 OFFSET %1)");

    SqlQueryPtr results = db->exec(limitQuery.arg(CFG_CORE.Console.HistorySize.get()));
    if (results->isError())
        qWarning() << "Error while limiting CLI history:" << db->getErrorText();
}

void ConfigImpl::asyncClearCliHistory()
{

}

void ConfigImpl::asyncAddDdlHistory()
{

}

void ConfigImpl::asyncClearDdlHistory()
{

}

void ConfigImpl::refreshSqlHistory()
{
    if (sqlHistoryModel)
        dynamic_cast<SqlHistoryModel*>(sqlHistoryModel)->refresh();
}
