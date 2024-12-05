#include "configimpl.h"
#include "sqlhistorymodel.h"
#include "ddlhistorymodel.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
#include "db/dbsqlite3.h"
#include "common/utils.h"
#include <QtGlobal>
#include <QDebug>
#include <QList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>
#include <QDataStream>
#include <QRegularExpression>
#include <QDateTime>
#include <QSysInfo>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QtWidgets/QFileDialog>

static_qstring(DB_FILE_NAME, "settings3");
static_qstring(CONFIG_DIR_SETTING, "SQLiteStudioConfigDir");
qint64 ConfigImpl::sqlHistoryId = -1;
QString ConfigImpl::memoryDbName = QStringLiteral(":memory:");

ConfigImpl::~ConfigImpl()
{
    cleanUp();
}

void ConfigImpl::init()
{
    initDbFile();
    initTables();
    updateConfigDb();
    mergeMasterConfig();

    sqlite3Version = db->exec("SELECT sqlite_version()")->getSingleCell().toString();

    connect(this, SIGNAL(sqlHistoryRefreshNeeded()), this, SLOT(refreshSqlHistory()));
    connect(this, SIGNAL(ddlHistoryRefreshNeeded()), this, SLOT(refreshDdlHistory()));
}

void ConfigImpl::cleanUp()
{
    if (db->isOpen())
        db->close();

    safe_delete(db)
}

const QString &ConfigImpl::getConfigDir() const
{
    return configDir;
}

QString ConfigImpl::getConfigFilePath() const
{
    if (!db)
        return QString();

    return db->getPath();
}

bool ConfigImpl::isInMemory() const
{
    return db->getPath() == memoryDbName;
}

void ConfigImpl::beginMassSave()
{
    if (isMassSaving())
        return;

    emit massSaveBegins();
    db->exec("BEGIN;");
    massSaving = true;
}

void ConfigImpl::commitMassSave()
{
    if (!isMassSaving())
        return;

    db->exec("COMMIT;");
    emit massSaveCommitted();
    massSaving = false;
}

void ConfigImpl::rollbackMassSave()
{
    if (!isMassSaving())
        return;

    db->exec("ROLLBACK;");
    massSaving = false;
}

bool ConfigImpl::isMassSaving() const
{
    return massSaving;
}

void ConfigImpl::set(const QString &group, const QString &key, const QVariant &value)
{
    db->exec("INSERT OR REPLACE INTO settings VALUES (?, ?, ?)", {group, key, serializeToBytes(value, dataStreamVersion)});
}

QVariant ConfigImpl::get(const QString &group, const QString &key)
{
    SqlQueryPtr results = db->exec("SELECT value FROM settings WHERE [group] = ? AND [key] = ?", {group, key});
    return deserializeValue(results->getSingleCell());
}

QVariant ConfigImpl::get(const QString &group, const QString &key, const QVariant &defaultValue)
{
    QVariant value = get(group, key);
    if (!value.isValid() || value.isNull())
        return defaultValue;

    return value;
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
    QByteArray optBytes = hashToBytes(options);
    SqlQueryPtr results = db->exec("INSERT INTO dblist VALUES (?, ?, ?)", {name, path, optBytes});
    return !storeErrorAndReturn(results);
}

bool ConfigImpl::updateDb(const QString &name, const QString &newName, const QString &path, const QHash<QString,QVariant> &options)
{
    QByteArray optBytes = hashToBytes(options);
    SqlQueryPtr results = db->exec("UPDATE dblist SET name = ?, path = ?, options = ? WHERE name = ?",
                                     {newName, path, optBytes, name});

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

QString ConfigImpl::getSqlite3Version() const
{
    return sqlite3Version;
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
        cfgDb->options = deserializeValue(row->value("options")).toHash();
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
    cfgDb->options = deserializeValue(row->value("options")).toHash();
    return cfgDb;
}

void ConfigImpl::storeGroups(const QList<DbGroupPtr>& groups)
{
    db->begin();
    db->exec("DELETE FROM groups");

    for (const DbGroupPtr& group : groups)
        storeGroup(group);

    db->commit();
}

void ConfigImpl::storeGroup(const ConfigImpl::DbGroupPtr &group, qint64 parentId)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVariant parent = QVariant(QVariant::LongLong);
#else
    QVariant parent = QVariant(QMetaType::fromType<qlonglong>());
#endif
    if (parentId > -1)
        parent = parentId;

    SqlQueryPtr results = db->exec("INSERT INTO groups (name, [order], parent, open, dbname, db_expanded) VALUES (?, ?, ?, ?, ?, ?)",
                                    {group->name, group->order, parent, group->open, group->referencedDbName, group->dbExpanded});

    qint64 newParentId = results->getRegularInsertRowId();
    for (const DbGroupPtr& childGroup : group->childs)
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
    SqlQueryPtr results = db->exec("SELECT id, name, [order], open, dbname, db_expanded FROM groups WHERE dbname = ? LIMIT 1", {dbName});

    DbGroupPtr group = DbGroupPtr::create();
    group->referencedDbName = dbName;

    if (!results->hasNext())
        return group;

    SqlResultsRowPtr row = results->next();
    group->id = row->value("id").toULongLong();
    group->name = row->value("name").toString();
    group->order = row->value("order").toInt();
    group->open = row->value("open").toBool();
    group->dbExpanded = row->value("db_expanded").toBool();
    return group;
}

qint64 ConfigImpl::addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    if (sqlHistoryId < 0)
    {
        SqlQueryPtr results = db->exec("SELECT max(id) FROM sqleditor_history");
        if (results->isError())
        {
            qCritical() << "Cannot add SQL history, because cannot determinate sqleditor_history max(id):" << results->getErrorText();
            return -1;
        }

        if (results->hasNext())
            sqlHistoryId = results->getSingleCell().toLongLong() + 1;
        else
            sqlHistoryId = 0;
    }

    sqlHistoryMutex.lock();
    runInThread([=, this]{ asyncAddSqlHistory(sqlHistoryId, sql, dbName, timeSpentMillis, rowsAffected); });
    return sqlHistoryId++;
}

void ConfigImpl::updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    sqlHistoryMutex.lock();
    runInThread([=, this]{ asyncUpdateSqlHistory(id, sql, dbName, timeSpentMillis, rowsAffected); });
}

void ConfigImpl::clearSqlHistory()
{
    runInThread([=, this]{ asyncClearSqlHistory(); });
}

void ConfigImpl::deleteSqlHistory(const QList<qint64>& ids)
{
    runInThread([=, this]{ asyncDeleteSqlHistory(ids); });
}

QAbstractItemModel* ConfigImpl::getSqlHistoryModel()
{
    if (!sqlHistoryModel)
        sqlHistoryModel = new SqlHistoryModel(db, this);

    return sqlHistoryModel;
}

void ConfigImpl::addCliHistory(const QString& text)
{
    runInThread([=, this]{ asyncAddCliHistory(text); });
}

void ConfigImpl::applyCliHistoryLimit()
{
    runInThread([=, this]{ asyncApplyCliHistoryLimit(); });
}

void ConfigImpl::clearCliHistory()
{
    runInThread([=, this]{ asyncClearCliHistory(); });
}

QStringList ConfigImpl::getCliHistory() const
{
    static_qstring(selectQuery, "SELECT text FROM cli_history ORDER BY id");

    SqlQueryPtr results = db->exec(selectQuery);
    if (results->isError())
        qWarning() << "Error while getting CLI history:" << db->getErrorText();

    return results->columnAsList<QString>("text");
}

void ConfigImpl::addBindParamHistory(const QVector<QPair<QString, QVariant> >& params)
{
    runInThread([=, this]{ asyncAddBindParamHistory(params); });
}

void ConfigImpl::applyBindParamHistoryLimit()
{
    runInThread([=, this]{ asyncApplyBindParamHistoryLimit(); });
}

QVector<QPair<QString, QVariant>> ConfigImpl::getBindParamHistory(const QStringList& paramNames) const
{
    static_qstring(directQuery, "SELECT id FROM bind_params WHERE pattern = ? ORDER BY id DESC");
    static_qstring(paramsByIdQuery, "SELECT name, value FROM bind_param_values WHERE bind_params_id = ? ORDER BY position");
    static_qstring(singleParamQuery, "SELECT value FROM bind_param_values WHERE %1 = ? ORDER BY id DESC LIMIT 1;");
    static_qstring(singleParamName, "name");
    static_qstring(singleParamPosition, "position");

    QVector<QPair<QString, QVariant>> bindParams;
    bindParams.reserve(paramNames.size());

    SqlQueryPtr results = db->exec(directQuery, {paramNames.join(",")});
    if (results->isError())
    {
        qWarning() << "Error while getting BindParams (1):" << db->getErrorText();
        return bindParams;
    }

    // Got an exact match? Extract values and return.
    QVariant exactMatch = results->getSingleCell();
    if (!exactMatch.isNull())
    {
        results = db->exec(paramsByIdQuery, {exactMatch.toLongLong()});
        if (results->isError())
        {
            qWarning() << "Error while getting BindParams (2):" << db->getErrorText();
        }
        else
        {
            for (const SqlResultsRowPtr& row : results->getAll())
                bindParams << QPair<QString, QVariant>(row->value("name").toString(), row->value("value"));
        }
        return bindParams;
    }

    // No exact match. Will look for values one by one using param name and position.
    int position = 0;
    for (const QString& bindParam : paramNames)
    {
        if (bindParam == "?")
            results = db->exec(singleParamQuery.arg(singleParamPosition), {position});
        else
            results = db->exec(singleParamQuery.arg(singleParamName), {bindParam});

        bindParams << QPair<QString, QVariant>(bindParam, results->getSingleCell());
        position++;
    }
    return bindParams;
}

void ConfigImpl::addPopulateHistory(const QString& database, const QString& table, int rows, const QHash<QString, QPair<QString, QVariant> >& columnsPluginsConfig)
{
    runInThread([=, this]{ asyncAddPopulateHistory(database, table, rows, columnsPluginsConfig); });
}

void ConfigImpl::applyPopulateHistoryLimit()
{
    runInThread([=, this]{ asyncApplyPopulateHistoryLimit(); });
}

QHash<QString, QPair<QString, QVariant>> ConfigImpl::getPopulateHistory(const QString& database, const QString& table, int& rows) const
{
    static_qstring(initialQuery, "SELECT id, rows FROM populate_history WHERE [database] = ? AND [table] = ? ORDER BY id DESC LIMIT 1");
    static_qstring(columnsQuery, "SELECT column_name, plugin_name, plugin_config FROM populate_column_history WHERE populate_history_id = ?");

    QHash<QString, QPair<QString, QVariant>> historyEntry;
    SqlQueryPtr results = db->exec(initialQuery, {database, table});
    if (results->isError())
    {
        qWarning() << "Error while getting Populating history entry (1):" << db->getErrorText();
        return historyEntry;
    }

    if (!results->hasNext())
        return historyEntry;

    SqlResultsRowPtr row = results->next();
    qint64 historyEntryId = row->value("id").toLongLong();
    rows = row->value("rows").toInt();

    results = db->exec(columnsQuery, {historyEntryId});
    QVariant value;
    while (results->hasNext())
    {
        row = results->next();
        value = deserializeValue(row->value("plugin_config"));
        historyEntry[row->value("column_name").toString()] = QPair<QString, QVariant>(row->value("plugin_name").toString(), value);
    }

    return historyEntry;
}

QVariant ConfigImpl::getPopulateHistory(const QString& pluginName) const
{
    static_qstring(columnsQuery, "SELECT plugin_config FROM populate_column_history WHERE plugin_name = ? ORDER BY id DESC LiMIT 1");

    SqlQueryPtr results = db->exec(columnsQuery, {pluginName});
    if (results->isError())
    {
        qWarning() << "Error while getting Populating history entry (2):" << db->getErrorText();
        return QVariant();
    }

    return deserializeValue(results->getSingleCell());
}

void ConfigImpl::addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile)
{
    ddlHistoryMutex.lock();
    runInThread([=, this]{ asyncAddDdlHistory(queries, dbName, dbFile); });
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

    QList<DdlHistoryEntryPtr> entries;
    DdlHistoryEntryPtr entry;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        entry = DdlHistoryEntryPtr::create();
        entry->dbName = dbName;
        entry->dbFile = dbFile;
        entry->timestamp = QDateTime::fromSecsSinceEpoch(row->value("timestamp").toLongLong());
        entry->queries = row->value("queries").toString();
        entries << entry;
    }
    return entries;
}

DdlHistoryModel* ConfigImpl::getDdlHistoryModel()
{
    if (!ddlHistoryModel)
        ddlHistoryModel = new DdlHistoryModel(db, this);

    return ddlHistoryModel;
}

void ConfigImpl::clearDdlHistory()
{
    runInThread([=, this]{ asyncClearDdlHistory(); });
}

void ConfigImpl::addReportHistory(bool isFeatureRequest, const QString& title, const QString& url)
{
    runInThread([=, this]{ asyncAddReportHistory(isFeatureRequest, title, url); });
}

QList<Config::ReportHistoryEntryPtr> ConfigImpl::getReportHistory()
{
    static_qstring(sql, "SELECT id, timestamp, title, url, feature_request FROM reports_history");

    SqlQueryPtr results = db->exec(sql);

    QList<ReportHistoryEntryPtr> entries;
    SqlResultsRowPtr row;
    ReportHistoryEntryPtr entry;
    while (results->hasNext())
    {
        row = results->next();
        entry = ReportHistoryEntryPtr::create();
        entry->id = row->value("id").toInt();
        entry->timestamp = row->value("timestamp").toInt();
        entry->title = row->value("title").toString();
        entry->url = row->value("url").toString();
        entry->isFeatureRequest = row->value("feature_request").toBool();
        entries << entry;
    }
    return entries;
}

void ConfigImpl::deleteReport(int id)
{
    runInThread([=, this]{ asyncDeleteReport(id); });
}

void ConfigImpl::clearReportHistory()
{
    runInThread([=, this]{ asyncClearReportHistory(); });
}

void ConfigImpl::readGroupRecursively(ConfigImpl::DbGroupPtr group)
{
    SqlQueryPtr results;
    if (group->id < 0)
        results = db->exec("SELECT id, name, [order], open, dbname, db_expanded FROM groups WHERE parent IS NULL ORDER BY [order]");
    else
        results = db->exec("SELECT id, name, [order], open, dbname, db_expanded FROM groups WHERE parent = ? ORDER BY [order]", {group->id});

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
        childGroup->dbExpanded = row->value("db_expanded").toBool();
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
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/" + DB_FILE_NAME;
}

QString ConfigImpl::getLegacyConfigPath()
{
#ifdef Q_OS_WIN
    return SQLITESTUDIO->getEnv("APPDATA")+"/sqlitestudio";
#else
    return SQLITESTUDIO->getEnv("HOME")+"/.config/sqlitestudio";
#endif
}

void ConfigImpl::dropTables(const QList<QString>& tables)
{
    for (const QString& table : tables)
        db->exec("DROP TABLE " + table);
}

void ConfigImpl::initTables()
{
    SqlQueryPtr results = db->exec("SELECT lower(name) AS name FROM sqlite_master WHERE type = 'table'");
    QList<QString> tables = results->columnAsList<QString>(0);

    if (!tables.contains("version"))
    {
        dropTables(tables);
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
                 "db_expanded INTEGER DEFAULT 0, UNIQUE(name, parent))");

    if (!tables.contains("ddl_history"))
        db->exec("CREATE TABLE ddl_history (id INTEGER PRIMARY KEY AUTOINCREMENT, dbname TEXT, file TEXT, timestamp INTEGER, "
                 "queries TEXT)");

    if (!tables.contains("cli_history"))
        db->exec("CREATE TABLE cli_history (id INTEGER PRIMARY KEY AUTOINCREMENT, text TEXT)");

    if (!tables.contains("reports_history"))
        db->exec("CREATE TABLE reports_history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, feature_request BOOLEAN, title TEXT, url TEXT)");

    if (!tables.contains("bind_params"))
    {
        db->exec("CREATE TABLE bind_params (id INTEGER PRIMARY KEY AUTOINCREMENT, pattern TEXT NOT NULL)");
        db->exec("CREATE INDEX bind_params_patt_idx ON bind_params (pattern);");
    }

    if (!tables.contains("bind_param_values"))
    {
        db->exec("CREATE TABLE bind_param_values (id INTEGER PRIMARY KEY AUTOINCREMENT, bind_params_id INTEGER REFERENCES bind_params (id) "
                 "ON DELETE CASCADE ON UPDATE CASCADE NOT NULL, position INTEGER NOT NULL, name TEXT NOT NULL, value)");
        db->exec("CREATE INDEX bind_param_values_fk_idx ON bind_param_values (bind_params_id);");
    }

    if (!tables.contains("populate_history"))
    {
        db->exec("CREATE TABLE populate_history (id INTEGER PRIMARY KEY AUTOINCREMENT, [database] TEXT NOT NULL, [table] TEXT NOT NULL, rows INTEGER NOT NULL)");
    }

    if (!tables.contains("populate_column_history"))
    {
        db->exec("CREATE TABLE populate_column_history (id INTEGER PRIMARY KEY AUTOINCREMENT, populate_history_id INTEGER REFERENCES populate_history (id) "
                 "ON DELETE CASCADE ON UPDATE CASCADE NOT NULL, column_name TEXT NOT NULL, plugin_name TEXT NOT NULL, plugin_config BLOB)");
        db->exec("CREATE INDEX populate_plugin_history_idx ON populate_column_history (plugin_name)");
    }

    if (!tables.contains("reports_history"))
        db->exec("CREATE TABLE reports_history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, feature_request BOOLEAN, title TEXT, url TEXT)");

    if (!tables.contains("script_functions"))
        db->exec("CREATE TABLE script_functions (name TEXT, lang TEXT, code TEXT, \"initCode\" TEXT, \"finalCode\" TEXT, databases TEXT, arguments TEXT,"
                 " \"type\" INTEGER, \"undefinedArgs\" BOOLEAN, \"allDatabases\" BOOLEAN, deterministic BOOLEAN)");
}

void ConfigImpl::initDbFile()
{
    QList<ConfigDirCandidate> paths = getStdDbPaths();

    // A fallback to in-memory db
    paths << ConfigDirCandidate{memoryDbName, false, false};

    // Go through all candidates and pick one
    QDir dir;
    for (ConfigDirCandidate& path : paths)
    {
        dir = QDir(path.path);
        if (path.path != memoryDbName)
            dir = QFileInfo(path.path).dir();

        if (tryInitDbFile(path))
        {
            configDir = dir.absolutePath();
            break;
        }
    }

    // If not one of std paths, look for the one from previously saved.
    if (configDir == memoryDbName)
    {
        // Do we have path selected by user? (because app was unable to find writable location before)
        QSettings* sett = getSettings();
        QString path = sett->value(CONFIG_DIR_SETTING).toString();
        if (!path.isEmpty())
        {
            paths << ConfigDirCandidate{path + "/" + DB_FILE_NAME, false, false};
            qDebug() << "Using custom configuration directory. The location is stored in" << sett->fileName();
        }
    }

    // Failed to use any of predefined directories. Let's ask the user.
    while (configDir == memoryDbName)
    {
        QString path = askUserForConfigDirFunc();
        if (path.isNull())
            break;

        dir = QDir(path);
        if (tryInitDbFile(ConfigDirCandidate{path + "/" + DB_FILE_NAME, false, false}))
        {
            configDir = dir.absolutePath();
            QSettings* sett = getSettings();
            sett->setValue(CONFIG_DIR_SETTING, configDir);
            qDebug() << "Using custom configuration directory. The location is stored in" << sett->fileName();
        }
    }

    // We ended up with in-memory one? That's not good.
    if (configDir == memoryDbName)
    {
        paths.removeLast();
        QStringList pathStrings;
        for (ConfigDirCandidate& path : paths)
            pathStrings << path.path;

        notifyError(QObject::tr("Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart."
                       " Unable to create a file at following locations: %1.").arg(pathStrings.join(", ")));
    }

    qDebug().noquote() << "Using configuration directory:" << toNativePath(configDir);
    db->exec("PRAGMA foreign_keys = 1;");
}

QList<ConfigImpl::ConfigDirCandidate> ConfigImpl::getStdDbPaths()
{
    QList<ConfigDirCandidate> paths;

    // Portable dir location has always precedense - comes first
    QString portablePath = getPortableConfigPath();
    if (!portablePath.isNull())
        paths << ConfigDirCandidate{portablePath+"/"+DB_FILE_NAME, false, true};

    // Determinate global config location
    QString globalPath = getConfigPath();
    paths << ConfigDirCandidate{globalPath, true, false};

    // If needed, migrate configuration from legacy location (pre-3.3) to new location (3.3 and later)
    QString legacyGlobalPath = getLegacyConfigPath();
    if (!legacyGlobalPath.isNull())
    {
        paths << ConfigDirCandidate{legacyGlobalPath+"/"+DB_FILE_NAME, true, false};
        if (!QFile::exists(globalPath))
            tryToMigrateOldGlobalPath(legacyGlobalPath, globalPath);
    }

    return paths;
}

bool ConfigImpl::tryInitDbFile(const ConfigDirCandidate& dbPath)
{
    // Create global config directory if not existing
    if (dbPath.createIfNotExists && !dbPath.path.isNull())
    {
        QDir dir(dbPath.path.mid(0, dbPath.path.length() - DB_FILE_NAME.length() - 1));
        if (!dir.exists())
            QDir::root().mkpath(dir.absolutePath());
    }

    db = new DbSqlite3("SQLiteStudio settings", dbPath.path, {{DB_PURE_INIT, true}});
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

QVariant ConfigImpl::deserializeValue(const QVariant &value) const
{
    if (!value.isValid())
        return QVariant();

    QByteArray bytes = value.toByteArray();
    return deserializeFromBytes(bytes, dataStreamVersion);
}

void ConfigImpl::asyncAddSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    db->begin();
    SqlQueryPtr results = db->exec("INSERT INTO sqleditor_history (id, dbname, date, time_spent, rows, sql) VALUES (?, ?, ?, ?, ?, ?)",
                                    {id, dbName, (QDateTime::currentMSecsSinceEpoch() / 1000), timeSpentMillis, rowsAffected, sql});

    if (results->isError())
    {
        qDebug() << "Error adding SQL history:" << results->getErrorText();
        db->rollback();
        sqlHistoryMutex.unlock();
        return;
    }

    int maxHistorySize = CFG_CORE.General.SqlHistorySize.get();

    results = db->exec("SELECT count(*) FROM sqleditor_history");
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
    db->commit();

    emit sqlHistoryRefreshNeeded();
    sqlHistoryMutex.unlock();
}

void ConfigImpl::asyncUpdateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    db->exec("UPDATE sqleditor_history SET dbname = ?, time_spent = ?, rows = ?, sql = ? WHERE id = ?",
            {dbName, timeSpentMillis, rowsAffected, sql, id});

    emit sqlHistoryRefreshNeeded();
    sqlHistoryMutex.unlock();
}

void ConfigImpl::asyncClearSqlHistory()
{
    db->exec("DELETE FROM sqleditor_history");
    emit sqlHistoryRefreshNeeded();
}

void ConfigImpl::asyncDeleteSqlHistory(const QList<qint64>& ids)
{
    if (!db->begin()) {
        NOTIFY_MANAGER->warn(tr("Could not start database transaction for deleting SQL history, therefore it's not deleted."));
        return;
    }
    for (const qint64& id : ids)
        db->exec("DELETE FROM sqleditor_history WHERE id = ?", id);

    if (!db->commit()) {
        NOTIFY_MANAGER->warn(tr("Could not commit database transaction for deleting SQL history, therefore it's not deleted."));
        db->rollback();
        return;
    }
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
    static_qstring(limitQuery, "DELETE FROM cli_history WHERE id <= (SELECT id FROM cli_history ORDER BY id DESC LIMIT 1 OFFSET %1)");

    SqlQueryPtr results = db->exec(limitQuery.arg(CFG_CORE.Console.HistorySize.get()));
    if (results->isError())
        qWarning() << "Error while limiting CLI history:" << db->getErrorText();
}

void ConfigImpl::asyncClearCliHistory()
{
    static_qstring(clearQuery, "DELETE FROM cli_history");

    SqlQueryPtr results = db->exec(clearQuery);
    if (results->isError())
        qWarning() << "Error while clearing CLI history:" << db->getErrorText();
}

void ConfigImpl::asyncAddBindParamHistory(const QVector<QPair<QString, QVariant> >& params)
{
    static_qstring(insertParamsQuery, "INSERT INTO bind_params (pattern) VALUES (?)");
    static_qstring(insertValuesQuery, "INSERT INTO bind_param_values (bind_params_id, position, name, value) VALUES (?, ?, ?, ?)");

    if (!db->begin())
    {
        qWarning() << "Failed to store BindParam cache, because could not begin SQL transaction. Details:" << db->getErrorText();
        return;
    }

    QStringList paramNames;
    for (const QPair<QString, QVariant>& paramPair : params)
        paramNames << paramPair.first;

    SqlQueryPtr results = db->exec(insertParamsQuery, {paramNames.join(",")});
    RowId rowId = results->getInsertRowId();
    qint64 bindParamsId = rowId["ROWID"].toLongLong();

    int position = 0;
    for (const QPair<QString, QVariant>& paramPair : params)
    {
        results = db->exec(insertValuesQuery, {bindParamsId, position++, paramPair.first, paramPair.second});
        if (results->isError())
        {
            qWarning() << "Failed to store BindParam cache, due to SQL error:" << db->getErrorText();
            db->rollback();
            return;
        }
    }

    if (!db->commit())
    {
        qWarning() << "Failed to store BindParam cache, because could not commit SQL transaction. Details:" << db->getErrorText();
        db->rollback();
    }

    asyncApplyBindParamHistoryLimit();
}

void ConfigImpl::asyncApplyBindParamHistoryLimit()
{
    static_qstring(findBindParamIdQuery, "SELECT bind_params_id FROM bind_param_values ORDER BY id DESC LIMIT 1 OFFSET %1");
    static_qstring(limitBindParamsQuery, "DELETE FROM bind_params WHERE id <= ?"); // will cascade with FK to bind_param_values

    SqlQueryPtr results = db->exec(findBindParamIdQuery.arg(CFG_CORE.General.BindParamsCacheSize.get()));
    if (results->isError())
        qWarning() << "Error while limiting BindParam history (step 1):" << db->getErrorText();

    qint64 bindParamId = results->getSingleCell().toLongLong();
    results = db->exec(limitBindParamsQuery, {bindParamId});
    if (results->isError())
        qWarning() << "Error while limiting BindParam history (step 2):" << db->getErrorText();
}

void ConfigImpl::asyncAddPopulateHistory(const QString& database, const QString& table, int rows, const QHash<QString, QPair<QString, QVariant>>& columnsPluginsConfig)
{
    static_qstring(insertQuery, "INSERT INTO populate_history ([database], [table], rows) VALUES (?, ?, ?)");
    static_qstring(insertColumnQuery, "INSERT INTO populate_column_history (populate_history_id, column_name, plugin_name, plugin_config) VALUES (?, ?, ?, ?)");

    if (!db->begin())
    {
        qWarning() << "Failed to store Populating history entry, because could not begin SQL transaction. Details:" << db->getErrorText();
        return;
    }

    SqlQueryPtr results = db->exec(insertQuery, {database, table, rows});
    RowId rowId = results->getInsertRowId();
    qint64 populateHistoryId = rowId["ROWID"].toLongLong();

    for (QHash<QString, QPair<QString, QVariant>>::const_iterator colIt = columnsPluginsConfig.begin(); colIt != columnsPluginsConfig.end(); colIt++)
    {
        results = db->exec(insertColumnQuery, {populateHistoryId, colIt.key(), colIt.value().first, serializeToBytes(colIt.value().second, dataStreamVersion)});
        if (results->isError())
        {
            qWarning() << "Failed to store Populating history entry, due to SQL error:" << db->getErrorText();
            db->rollback();
            return;
        }
    }

    if (!db->commit())
    {
        qWarning() << "Failed to store Populating history entry, because could not commit SQL transaction. Details:" << db->getErrorText();
        db->rollback();
    }

    asyncApplyPopulateHistoryLimit();
}

void ConfigImpl::asyncApplyPopulateHistoryLimit()
{
    static_qstring(limitQuery, "DELETE FROM populate_history WHERE id <= (SELECT id FROM populate_history ORDER BY id DESC LIMIT 1 OFFSET %1)");

    SqlQueryPtr results = db->exec(limitQuery.arg(CFG_CORE.General.PopulateHistorySize.get()));
    if (results->isError())
        qWarning() << "Error while limiting Populating history:" << db->getErrorText();
}

void ConfigImpl::asyncAddDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile)
{
    static_qstring(insert, "INSERT INTO ddl_history (dbname, file, timestamp, queries) VALUES (?, ?, ?, ?)");
    static_qstring(countSql, "SELECT count(*) FROM ddl_history");
    static_qstring(idSql, "SELECT id FROM ddl_history ORDER BY id DESC LIMIT 1 OFFSET %1");
    static_qstring(deleteSql, "DELETE FROM ddl_history WHERE id <= ?");

    db->begin();
    db->exec(insert, {dbName, dbFile, QDateTime::currentDateTime().toSecsSinceEpoch(), queries});

    int maxHistorySize = CFG_CORE.General.DdlHistorySize.get();

    SqlQueryPtr results = db->exec(countSql);
    if (results->hasNext() && results->getSingleCell().toInt() > maxHistorySize)
    {
        results = db->exec(QString(idSql).arg(maxHistorySize), Db::Flag::NO_LOCK);
        if (results->hasNext())
        {
            int id = results->getSingleCell().toInt();
            if (id > 0) // it will be 0 on fail conversion, but we won't delete id <= 0 ever.
                db->exec(deleteSql, {id});
        }
    }
    db->commit();
    ddlHistoryMutex.unlock();

    emit ddlHistoryRefreshNeeded();
}

void ConfigImpl::asyncClearDdlHistory()
{
    db->exec("DELETE FROM ddl_history");
    emit ddlHistoryRefreshNeeded();
}

void ConfigImpl::asyncAddReportHistory(bool isFeatureRequest, const QString& title, const QString& url)
{
    static_qstring(sql, "INSERT INTO reports_history (feature_request, timestamp, title, url) VALUES (?, ?, ?, ?)");
    db->exec(sql, {(isFeatureRequest ? 1 : 0), QDateTime::currentDateTime().toSecsSinceEpoch(), title, url});
    emit reportsHistoryRefreshNeeded();
}

void ConfigImpl::asyncDeleteReport(int id)
{
    static_qstring(sql, "DELETE FROM reports_history WHERE id = ?");
    db->exec(sql, {id});
    emit reportsHistoryRefreshNeeded();
}

void ConfigImpl::asyncClearReportHistory()
{
    static_qstring(sql, "DELETE FROM reports_history");
    db->exec(sql);
    emit reportsHistoryRefreshNeeded();
}

void ConfigImpl::mergeMasterConfig()
{
    QString masterConfigFile = Config::getMasterConfigFile();
    if (masterConfigFile.isEmpty())
        return;

#if QT_VERSION >= 0x050500
    qInfo()
#else
    qDebug()
#endif
        << "Updating settings from master configuration file: " << masterConfigFile;

    Db* masterDb = new DbSqlite3("SQLiteStudio master settings", masterConfigFile, {{DB_PURE_INIT, true}});
    if (!masterDb->open())
    {
        safe_delete(masterDb);
        qWarning() << "Could not open master config database:" << masterConfigFile;
        return;
    }

    SqlQueryPtr results = masterDb->exec("SELECT [group], key, value FROM settings");
    if (results->isError())
    {
        qWarning() << "Could not query master config database:" << masterConfigFile << ", error details:" << results->getErrorText();
        safe_delete(masterDb);
        return;
    }

    static_qstring(insertSql, "INSERT OR IGNORE INTO settings ([group], key, value) VALUES (?, ?, ?)");
    db->begin();
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        if (row->value("group") == "General" && row->value("key") == "Session")
            continue; // Don't copy session

        db->exec(insertSql, row->valueList());
    }
    db->commit();

    masterDb->close();
    safe_delete(masterDb);
}

void ConfigImpl::updateConfigDb()
{
    SqlQueryPtr result = db->exec("SELECT version FROM version LIMIT 1");
    int dbVersion = result->getSingleCell().toInt();
    if (dbVersion >= SQLITESTUDIO_CONFIG_VERSION)
        return;

    db->begin();
    switch (dbVersion)
    {
        case 1:
        {
            // 1->2
            db->exec("UPDATE settings SET [key] = 'DataUncommittedError' WHERE [key] = 'DataUncommitedError'");
            db->exec("UPDATE settings SET [key] = 'DataUncommitted' WHERE [key] = 'DataUncommited'");
            Q_FALLTHROUGH();
        }
        case 2:
        {
            // 2->3
            db->exec("ALTER TABLE groups ADD db_expanded INTEGER DEFAULT 0");
            Q_FALLTHROUGH();
        }
        case 3:
        {
            // 3->4
            QVariant oldFuncs = get("Internal", "Functions");
            if (oldFuncs.isValid() && !oldFuncs.isNull())
            {
                for (const QVariant& var : oldFuncs.toList())
                {
                    QHash<QString, QVariant> fnHash = var.toHash();
                    db->exec("INSERT INTO script_functions"
                             " (name, lang, code, \"initCode\", \"finalCode\", databases, arguments, \"type\", \"undefinedArgs\", \"allDatabases\", deterministic)"
                             " VALUES (?, REPLACE(?, 'QtScript', 'JavaScript'), ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                             { fnHash["name"].toString(), fnHash["lang"].toString(),
                               fnHash["code"].toString(), fnHash["initCode"].toString(), fnHash["finalCode"].toString(),
                               QString::fromUtf8(QJsonDocument(QJsonArray::fromVariantList(fnHash["databases"].toList())).toJson(QJsonDocument::Compact)),
                               QString::fromUtf8(QJsonDocument(QJsonArray::fromVariantList(fnHash["arguments"].toList())).toJson(QJsonDocument::Compact)),
                               fnHash["type"].toInt(), fnHash["undefinedArgs"].toBool(), fnHash["allDatabases"].toBool(),
                               fnHash["deterministic"].toBool() });
                }
            }
            db->exec("DELETE FROM settings WHERE [group] = 'Internal' AND [key] = 'Functions'");
        }
        // Add cases here for next versions,
        // without a "break" instruction,
        // in order to update from certain
        // version to latest at once.
    }

    db->exec("UPDATE version SET version = ?", {SQLITESTUDIO_CONFIG_VERSION});
    db->commit();
}

bool ConfigImpl::tryToMigrateOldGlobalPath(const QString& oldPath, const QString& newPath)
{
    if (!QFileInfo::exists(oldPath))
        return false;

    qDebug().noquote() << "Attempting to migrate legacy config location" << toNativePath(oldPath) << "to new location" << toNativePath(newPath);
    QDir dir = QFileInfo(newPath).dir();
    if (!dir.exists())
        QDir::root().mkpath(dir.absolutePath());

    if (QFile::copy(oldPath, dir.absoluteFilePath(DB_FILE_NAME)))
    {
        qDebug() << "Migration successful. Renaming old location file so it has '.old' suffix.";
        if (QFile::rename(oldPath, oldPath+".old"))
            qDebug() << "Renaming successful.";
        else
            qDebug() << "Renaming did not work, but it's okay. It will just remain with original name there.";
    }
    else
        qDebug() << "Migration (copying) failed.";

    return true;
}

void ConfigImpl::refreshSqlHistory()
{
    if (sqlHistoryModel)
        sqlHistoryModel->refresh();
}

void ConfigImpl::refreshDdlHistory()
{
    if (ddlHistoryModel)
        ddlHistoryModel->refresh();
}

QList<QHash<QString, QVariant> > ConfigImpl::getScriptFunctions()
{
    QList<QHash<QString, QVariant> > list;
    SqlQueryPtr results = db->exec("SELECT * FROM script_functions");
    while (results->hasNext())
    {
        SqlResultsRowPtr row = results->next();
        QHash<QString, QVariant> fnHash = row->valueMap();
        fnHash["databases"] = QJsonDocument::fromJson(row->value("databases").toByteArray()).toVariant();
        fnHash["arguments"] = QJsonDocument::fromJson(row->value("arguments").toByteArray()).toVariant();
        list << fnHash;
    }
    return list;
}

void ConfigImpl::setScriptFunctions(const QList<QHash<QString, QVariant> >& newFunctions)
{
    db->begin();
    db->exec("DELETE FROM script_functions");
    for (const QHash<QString, QVariant>& fnHash : newFunctions)
    {
        db->exec("INSERT INTO script_functions"
                 " (name, lang, code, \"initCode\", \"finalCode\", databases, arguments,"
                 "  \"type\", \"undefinedArgs\", \"allDatabases\", deterministic)"
                 " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                 {fnHash["name"].toString(), fnHash["lang"].toString(),
                  fnHash["code"].toString(), fnHash["initCode"].toString(), fnHash["finalCode"].toString(),
                  QString::fromUtf8(QJsonDocument(QJsonArray::fromVariantList(fnHash["databases"].toList())).toJson(QJsonDocument::Compact)),
                  QString::fromUtf8(QJsonDocument(QJsonArray::fromVariantList(fnHash["arguments"].toList())).toJson(QJsonDocument::Compact)),
                  fnHash["type"], fnHash["undefinedArgs"], fnHash["allDatabases"], fnHash["deterministic"]});
    }
    db->commit();
}
