#include "config.h"
#include "sqlitestudio.h"
#include "sqlhistorymodel.h"
#include "ddlhistorymodel.h"
#include "notifymanager.h"
#include <QtGlobal>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QDataStream>
#include <QRegExp>
#include <QDateTime>
#include <QSysInfo>

CFG_DEFINE(Core)

static const QString DB_FILE_NAME = QStringLiteral("settings3");

Config::Config(QObject *parent) :
    QObject(parent)
{
}

Config::~Config()
{
    cleanUp();
}

void Config::init()
{
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "sqlitestudio_private_config"));
    initDbFile();
    initTables();
}

void Config::cleanUp()
{
    if (db->isOpen())
        db->close();

    delete db;
    db = nullptr;

    QSqlDatabase::removeDatabase("sqlitestudio_private_config");
}

const QString &Config::getConfigDir()
{
    return configDir;
}

void Config::beginMassSave()
{
    db->exec("BEGIN;");
}

void Config::commitMassSave()
{
    db->exec("COMMIT;");
    emit massSaveCommited();
}

void Config::rollbackMassSave()
{
    db->exec("ROLLBACK;");
}

void Config::set(const QString &group, const QString &key, const QVariant &value)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << value;

    QSqlQuery query(*db);
    query.prepare("INSERT OR REPLACE INTO settings VALUES (:group, :key, :value)");
    query.bindValue(":group", group);
    query.bindValue(":key", key);
    query.bindValue(":value", bytes);
    query.exec();
}

QVariant Config::get(const QString &group, const QString &key)
{
    QSqlQuery query(*db);
    query.prepare("SELECT value FROM settings WHERE [group] = :group AND [key] = :key");
    query.bindValue(":group", group);
    query.bindValue(":key", key);
    query.exec();
    if (!query.next())
        return QVariant();

    return deserializeValue(query.value(0));
}

QHash<QString,QVariant> Config::getAll()
{
    QSqlQuery query(*db);
    query.prepare("SELECT [group], [key], value FROM settings");
    query.exec();

    QHash<QString,QVariant> results;
    QString key;
    while (query.next())
    {
        key = query.value("group").toString() + "." + query.value("key").toString();
        results[key] = deserializeValue(query.value("value"));
    }
    return results;
}

bool Config::storeErrorAndReturn(const QSqlQuery& query)
{
    if (query.lastError().isValid())
    {
        lastQueryError = query.lastError().text();
        return true;
    }
    else
        return false;
}

void Config::printErrorIfSet(const QSqlQuery& query)
{
    if (!query.isValid() && query.lastError().isValid())
    {
        qCritical() << "Config error while executing query:" << query.lastQuery() << ";\nError details:" << query.lastError().text();
        storeErrorAndReturn(query);
    }
}

bool Config::addDb(const QString& name, const QString& path, const QHash<QString,QVariant>& options)
{
    QSqlQuery query(*db);
    query.prepare("INSERT INTO dblist VALUES (:name, :path, :options)");
    query.bindValue(":name", name);
    query.bindValue(":path", path);
    query.bindValue(":options", options);
    query.exec();
    return !storeErrorAndReturn(query);
}

bool Config::updateDb(const QString &name, const QString &newName, const QString &path, const QHash<QString,QVariant> &options)
{
    QSqlQuery query(*db);
    query.prepare("UPDATE dblist SET name = :newName, path = :path, options = :options WHERE name = :name");
    query.bindValue(":name", name);
    query.bindValue(":newName", newName);
    query.bindValue(":path", path);
    query.bindValue(":options", options);
    query.exec();
    return (!storeErrorAndReturn(query)  && query.numRowsAffected() > 0);
}

bool Config::removeDb(const QString &name)
{
    QSqlQuery query(*db);
    query.prepare("DELETE FROM dblist WHERE name = :name");
    query.bindValue(":name", name);
    query.exec();
    return (!storeErrorAndReturn(query) && query.numRowsAffected() > 0);
}

bool Config::isDbInConfig(const QString &name)
{
    QSqlQuery query(*db);
    query.prepare("SELECT * FROM dblist WHERE name = :name");
    query.bindValue(":name", name);
    query.exec();
    return (!storeErrorAndReturn(query) && query.next());
}

QString Config::getLastErrorString() const
{
    QString msg = db->lastError().text();
    if (msg.trimmed().isEmpty())
        return lastQueryError;

    return msg;
}

QList<Config::CfgDbPtr> Config::dbList()
{
    QList<CfgDbPtr> results;
    QSqlQuery query = db->exec("SELECT name, path, options FROM dblist");
    CfgDbPtr cfgDb;
    while (query.next())
    {
        cfgDb = CfgDbPtr::create();
        cfgDb->name = query.value("name").toString();
        cfgDb->path = query.value("path").toString();
        cfgDb->options = query.value("options").toHash();
        results += cfgDb;
    }

    return results;
}

Config::CfgDbPtr Config::getDb(const QString& dbName)
{
    QSqlQuery query(*db);
    query.prepare("SELECT path, options FROM dblist WHERE name = :name");
    query.bindValue(":name", dbName);
    query.exec();

    if (!query.next())
        return CfgDbPtr();

    CfgDbPtr cfgDb = CfgDbPtr::create();
    cfgDb->name = dbName;
    cfgDb->path = query.value("path").toString();
    cfgDb->options = query.value("options").toHash();
    return cfgDb;
}

void Config::storeGroups(const QList<DbGroupPtr>& groups)
{
    db->exec("DELETE FROM groups");

    const QVariant parentId = QVariant(QVariant::LongLong);
    foreach (const DbGroupPtr& group, groups)
        storeGroup(group, parentId);
}

void Config::storeGroup(const Config::DbGroupPtr &group, const QVariant& parentId)
{
    QSqlQuery query(*db);
    query.prepare("INSERT INTO groups (name, [order], parent, open, dbname) VALUES (:name, :order, :parent, :open, :dbname)");
    query.bindValue(":name", group->name);
    query.bindValue(":order", group->order);
    query.bindValue(":parent", parentId);
    query.bindValue(":open", group->open);
    query.bindValue(":dbname", group->referencedDbName);
    query.exec();

    QVariant newParentId = query.lastInsertId();
    foreach (const DbGroupPtr& childGroup, group->childs)
        storeGroup(childGroup, newParentId);
}

QList<Config::DbGroupPtr> Config::getGroups()
{
    DbGroupPtr topGroup = DbGroupPtr::create();
    topGroup->id = -1;
    readGroupRecursively(topGroup);
    return topGroup->childs;
}

Config::DbGroupPtr Config::getDbGroup(const QString& dbName)
{
    QSqlQuery query(*db);
    query.prepare("SELECT id, name, [order], open, dbname FROM groups WHERE dbname = :dbname LIMIT 1");
    query.bindValue(":dbname", dbName);
    query.exec();

    DbGroupPtr group = DbGroupPtr::create();
    group->referencedDbName = dbName;

    if (!query.next())
        return group;

    group->id = query.value("id").toULongLong();
    group->name = query.value("name").toString();
    group->order = query.value("order").toInt();
    group->open = query.value("open").toBool();
    return group;
}

qint64 Config::addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    // TODO move this to separate thread, because counting 10000 rows might be too expensive
    // just for the sake of adding new history entry.
    QSqlQuery query(*db);
    query.prepare("INSERT INTO sqleditor_history (dbname, date, time_spent, rows, sql) VALUES (:dbname, :date, :time_spent, :rows, :sql)");
    query.bindValue(":dbname", dbName);
    query.bindValue(":date", QDateTime::currentMSecsSinceEpoch() / 1000);
    query.bindValue(":time_spent", timeSpentMillis);
    query.bindValue(":rows", rowsAffected);
    query.bindValue(":sql", sql);
    query.exec();
    qint64 rowId = query.lastInsertId().toLongLong();

    int maxHistorySize = CFG_CORE.General.SqlHistorySize.get();

    query.exec("SELECT count(*) FROM sqleditor_history");
    if (query.next() && query.value(0).toInt() > maxHistorySize)
    {
        query.prepare("SELECT id FROM sqleditor_history ORDER BY id DESC LIMIT 1 OFFSET :length");
        query.bindValue(":length", maxHistorySize);
        query.exec();
        if (query.next())
        {
            int id = query.value(0).toInt();
            if (id > 0) // it will be 0 on fail conversion, but we won't delete id <= 0 ever.
            {
                query.prepare("DELETE FROM sqleditor_history WHERE id <= :id");
                query.bindValue(":id", id);
                query.exec();
            }
        }
    }

    if (sqlHistoryModel)
        dynamic_cast<SqlHistoryModel*>(sqlHistoryModel)->refresh();

    return rowId;
}

void Config::updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    // TODO Move to thread just like addSqlHistory, for the same reason.
    QSqlQuery query(*db);
    query.prepare("UPDATE sqleditor_history SET dbname = :dbname, time_spent = :time_spent, rows = :rows, sql = :sql WHERE id = :id");
    query.bindValue(":dbname", dbName);
    query.bindValue(":time_spent", timeSpentMillis);
    query.bindValue(":rows", rowsAffected);
    query.bindValue(":sql", sql);
    query.bindValue(":id", id);
    query.exec();

    if (sqlHistoryModel)
        dynamic_cast<SqlHistoryModel*>(sqlHistoryModel)->refresh();
}

void Config::clearSqlHistory()
{
    QSqlQuery query(*db);
    query.exec("DELETE FROM sqleditor_history");

    if (sqlHistoryModel)
        dynamic_cast<SqlHistoryModel*>(sqlHistoryModel)->refresh();
}

QAbstractItemModel* Config::getSqlHistoryModel()
{
    if (!sqlHistoryModel)
        sqlHistoryModel = new SqlHistoryModel(this, db);

    return sqlHistoryModel;
}

void Config::addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile)
{
    QSqlQuery query(*db);
    query.prepare("INSERT INTO ddl_history (dbname, file, timestamp, queries) VALUES (:dbname, :file, :timestamp, :queries)");
    query.bindValue(":dbname", dbName);
    query.bindValue(":file", dbFile);
    query.bindValue(":timestamp", QDateTime::currentDateTime().toTime_t());
    query.bindValue(":queries", queries);
    query.exec();

    int maxHistorySize = CFG_CORE.General.DdlHistorySize.get();

    query.exec("SELECT count(*) FROM ddl_history");
    if (query.next() && query.value(0).toInt() > maxHistorySize)
    {
        query.prepare("SELECT id FROM ddl_history ORDER BY id DESC LIMIT 1 OFFSET :length");
        query.bindValue(":length", maxHistorySize);
        query.exec();
        if (query.next())
        {
            int id = query.value(0).toInt();
            if (id > 0) // it will be 0 on fail conversion, but we won't delete id <= 0 ever.
            {
                query.prepare("DELETE FROM ddl_history WHERE id <= :id");
                query.bindValue(":id", id);
                query.exec();
            }
        }
    }

    if (ddlHistoryModel)
            dynamic_cast<DdlHistoryModel*>(ddlHistoryModel)->refresh();
}

QList<Config::DdlHistoryEntryPtr> Config::getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date)
{
    static const QString sql =
            "SELECT timestamp,"
            "       queries"
            "  FROM ddl_history"
            " WHERE dbname = :dbName"
            "   AND file = :dbFile"
            "   AND date(timestamp, 'unixepoch') = :date"
            ;

    QSqlQuery query(*db);
    query.prepare(sql);
    query.bindValue(":dbName", dbName);
    query.bindValue(":dbFile", dbFile);
    query.bindValue(":date", date.toString("yyyy-MM-dd"));
    query.exec();

    QList<DdlHistoryEntryPtr> results;
    DdlHistoryEntryPtr entry;
    while (query.next())
    {
        entry = DdlHistoryEntryPtr::create();
        entry->dbName = dbName;
        entry->dbFile = dbFile;
        entry->timestamp = QDateTime::fromTime_t(query.value("timestamp").toUInt());
        entry->queries = query.value("queries").toString();
        results << entry;
    }
    return results;
}

DdlHistoryModel* Config::getDdlHistoryModel()
{
    if (!ddlHistoryModel)
        ddlHistoryModel = new DdlHistoryModel(this, db);

    return ddlHistoryModel;
}

void Config::clearDdlHistory()
{
    QSqlQuery query(*db);
    query.exec("DELETE FROM ddl_history");

    if (ddlHistoryModel)
        ddlHistoryModel->refresh();
}

bool Config::setFunctions(const QList<FunctionManager::FunctionPtr>& functions)
{
    if (!db->transaction())
    {
        qCritical() << "Could not start trasaction on config database! Functions not modified.";
        return false;
    }

    QSqlQuery functionQuery(*db);
    QSqlQuery fnArgsQuery(*db);
    QSqlQuery funcToDbQuery(*db);

    functionQuery.prepare("DELETE FROM functions");
    functionQuery.exec();

    functionQuery.prepare("INSERT INTO functions (name, lang, initial_code, code, final_code, type, for_all_databases, undefined_args) "
                          "VALUES (:name, :lang, :initial_code, :code, :final_code, :type, :for_all_databases, :undefined_args)");

    fnArgsQuery.prepare("INSERT INTO function_args (function, name) VALUES (:function, :name)");

    funcToDbQuery.prepare("INSERT INTO func_to_db (function, dbname) VALUES (:function, :dbname)");

    foreach (const FunctionManager::FunctionPtr& func, functions)
    {
        functionQuery.bindValue(":name", func->name);
        functionQuery.bindValue(":lang", func->lang);
        functionQuery.bindValue(":initial_code", func->initCode);
        functionQuery.bindValue(":code", func->code);
        functionQuery.bindValue(":final_code", func->finalCode);
        functionQuery.bindValue(":type", FunctionManager::Function::typeString(func->type));
        functionQuery.bindValue(":for_all_databases", func->allDatabases);
        functionQuery.bindValue(":undefined_args", func->undefinedArgs);
        functionQuery.exec();
        printErrorIfSet(functionQuery);

        // Arguments
        if (!func->undefinedArgs)
        {
            foreach (const QString arg, func->arguments)
            {
                fnArgsQuery.bindValue(":function", func->name);
                fnArgsQuery.bindValue(":name", arg);
                fnArgsQuery.exec();
                printErrorIfSet(fnArgsQuery);
            }
        }

        // Databases
        if (!func->allDatabases)
        {
            foreach (const QString dbName, func->databases)
            {
                funcToDbQuery.bindValue(":function", func->name);
                funcToDbQuery.bindValue(":dbname", dbName);
                funcToDbQuery.exec();
                printErrorIfSet(funcToDbQuery);
            }
        }
    }

    if (!db->commit())
    {
        qCritical() << "Could not commit function list modifications. Functions not changed. Error message:" << db->lastError().text();
        db->rollback();
        return false;
    }
    return true;
}

QList<FunctionManager::FunctionPtr> Config::getFunctions() const
{
    // Read function arguments
    QHash<QString,QStringList> arguments;
    QSqlQuery query(*db);
    query.prepare("SELECT * FROM function_args");
    query.exec();

    while (query.next())
        arguments[query.value("function").toString()] << query.value("name").toString();

    // Read relations to databases
    QHash<QString,QStringList> databases;
    query.prepare("SELECT * FROM func_to_db");
    query.exec();

    while (query.next())
        databases[query.value("function").toString()] << query.value("dbname").toString();

    // Read functions themself
    query.prepare("SELECT * FROM functions");
    query.exec();

    QList<FunctionManager::FunctionPtr> funcList;
    FunctionManager::FunctionPtr func;
    while (query.next())
    {
        func = FunctionManager::FunctionPtr::create();
        func->name = query.value("name").toString();
        func->lang = query.value("lang").toString();
        func->initCode = query.value("initial_code").toString();
        func->code = query.value("code").toString();
        func->finalCode = query.value("final_code").toString();
        func->type = FunctionManager::Function::typeString(query.value("type").toString());
        func->allDatabases = query.value("for_all_databases").toBool();
        func->undefinedArgs = query.value("undefined_args").toBool();
        func->arguments = arguments[func->name];
        func->databases = databases[func->name];
        funcList << func;
    }
    return funcList;
}

void Config::readGroupRecursively(Config::DbGroupPtr group)
{
    QSqlQuery query(*db);
    if (group->id < 0)
        query.prepare("SELECT id, name, [order], open, dbname FROM groups WHERE parent IS NULL ORDER BY [order]");
    else
    {
        query.prepare("SELECT id, name, [order], open, dbname FROM groups WHERE parent = :parentId ORDER BY [order]");
        query.bindValue(":parentId", group->id);
    }
    query.exec();
    DbGroupPtr childGroup;
    while (query.next())
    {
        childGroup = DbGroupPtr::create();
        childGroup->id = query.value("id").toULongLong();
        childGroup->name = query.value("name").toString();
        childGroup->order = query.value("order").toInt();
        childGroup->open = query.value("open").toBool();
        childGroup->referencedDbName = query.value("dbname").toString();
        group->childs += childGroup;
    }

    for (int i = 0; i < group->childs.size(); i++)
        readGroupRecursively(group->childs[i]);
}

void Config::begin()
{
    db->transaction();
}

void Config::commit()
{
    db->commit();
}

void Config::rollback()
{
    db->rollback();
}

QString Config::getConfigPath()
{
#ifdef Q_OS_WIN
    if (QSysInfo::windowsVersion() & QSysInfo::WV_NT_based)
        return SQLiteStudio::getInstance()->getEnv("APPDATA")+"/sqlitestudio";
    else
        return SQLiteStudio::getInstance()->getEnv("HOME")+"/sqlitestudio";
#else
    return SQLiteStudio::getInstance()->getEnv("HOME")+"/.sqlitestudio";
#endif
}

QString Config::getPortableConfigPath()
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

void Config::initTables()
{
    QList<QString> tables;
    QSqlQuery query = db->exec("SELECT lower(name) AS name FROM sqlite_master WHERE type = 'table'");
    while (query.next())
        tables << query.value(0).toString();

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

    if (!tables.contains("functions"))
        db->exec("CREATE TABLE functions (name TEXT PRIMARY KEY, lang TEXT, initial_code TEXT, code TEXT, final_code TEXT, "
                 "type TEXT CHECK (type IN ('SCALAR', 'AGGREGATE')), for_all_databases BOOLEAN, undefined_args BOOLEAN)");

    if (!tables.contains("function_args"))
        db->exec("CREATE TABLE function_args (function TEXT REFERENCES functions (name) ON UPDATE CASCADE ON DELETE CASCADE, "
                 "name TEXT)");

    if (!tables.contains("func_to_db"))
    {
        db->exec("CREATE TABLE func_to_db (function TEXT REFERENCES functions (name) ON UPDATE CASCADE ON DELETE CASCADE, "
                 "dbname TEXT REFERENCES dblist (name) ON UPDATE CASCADE ON DELETE CASCADE)");
        db->exec("DROP TRIGGER IF EXISTS func_to_db_trig");
        db->exec("CREATE TRIGGER func_to_db_trig BEFORE INSERT ON func_to_db WHEN "
                 "(SELECT for_all_databases FROM functions WHERE name = new.function) = 1 "
                 "BEGIN "
                 "SELECT RAISE(ROLLBACK, 'Cannot assign function to database, because it is marked for all databases.'); "
                 "END");
    }
}

void Config::initDbFile()
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
        notifyError(tr("Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart."
                       " Tried to initialize the file at following localizations: %1.").arg(paths.join(", ")));
    }

    db->exec("PRAGMA foreign_keys = 1;");
}

bool Config::tryInitDbFile(const QString &dbPath)
{
    db->setDatabaseName(dbPath);
    if (db->open())
    {
        qDebug() << "Using config: " << dbPath;
        return true;
    }
    else
    {
        qDebug() << db->lastError().text();
    }
    return false;
}

QVariant Config::deserializeValue(const QVariant &value)
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
