#ifndef CONFIGIMPL_H
#define CONFIGIMPL_H

#include "coreSQLiteStudio_global.h"
#include "services/config.h"
#include "db/sqlquery.h"
#include <QMutex>

class AsyncConfigHandler;
class SqlHistoryModel;
class QSettings;

class API_EXPORT ConfigImpl : public Config
{
    Q_OBJECT

    friend class AsyncConfigHandler;

    public:
        virtual ~ConfigImpl();

        void init();
        void cleanUp();
        const QString& getConfigDir() const;
        QString getConfigFilePath() const;
        bool isInMemory() const;

        void beginMassSave();
        void commitMassSave();
        void rollbackMassSave();
        bool isMassSaving() const;
        void set(const QString& group, const QString& key, const QVariant& value);
        QVariant get(const QString& group, const QString& key);
        QVariant get(const QString& group, const QString& key, const QVariant& defaultValue);
        QHash<QString,QVariant> getAll();

        bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant> &options);
        bool updateDb(const QString& name, const QString &newName, const QString& path, const QHash<QString, QVariant> &options);
        bool removeDb(const QString& name);
        bool isDbInConfig(const QString& name);
        QString getLastErrorString() const;
        QString getSqlite3Version() const;

        /**
         * @brief Provides list of all registered databases.
         * @return List of database entries.
         *
         * Registered databases are those that user added to the application. They are not necessary valid or supported.
         * They can be inexisting or unsupported, but they are kept in registry in case user fixes file path,
         * or loads plugin to support it.
         */
        QList<CfgDbPtr> dbList();
        CfgDbPtr getDb(const QString& dbName);

        void storeGroups(const QList<DbGroupPtr>& groups);
        QList<DbGroupPtr> getGroups();
        DbGroupPtr getDbGroup(const QString& dbName);

        qint64 addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void clearSqlHistory();
        void deleteSqlHistory(const QList<qint64>& ids);
        QAbstractItemModel* getSqlHistoryModel();

        void addCliHistory(const QString& text);
        void applyCliHistoryLimit();
        void clearCliHistory();
        QStringList getCliHistory() const;

        void addBindParamHistory(const QVector<QPair<QString, QVariant>>& params);
        void applyBindParamHistoryLimit();
        QVector<QPair<QString, QVariant>> getBindParamHistory(const QStringList& paramNames) const;

        void addPopulateHistory(const QString& database, const QString& table, int rows, const QHash<QString, QPair<QString, QVariant>>& columnsPluginsConfig);
        void applyPopulateHistoryLimit();
        QHash<QString, QPair<QString, QVariant>> getPopulateHistory(const QString& database, const QString& table, int& rows) const;
        QVariant getPopulateHistory(const QString& pluginName) const;

        void addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile);
        QList<DdlHistoryEntryPtr> getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date);
        DdlHistoryModel* getDdlHistoryModel();
        void clearDdlHistory();

        void addReportHistory(bool isFeatureRequest, const QString& title, const QString& url);
        QList<ReportHistoryEntryPtr> getReportHistory();
        void deleteReport(int id);
        void clearReportHistory();

        QList<QHash<QString, QVariant> > getScriptFunctions();
        void setScriptFunctions(const QList<QHash<QString, QVariant> >& newFunctions);

        void begin();
        void commit();
        void rollback();

    private:
        struct ConfigDirCandidate
        {
            QString path;
            bool createIfNotExists;
            bool isPortable;
        };

        /**
         * @brief Stores error from query in class member.
         * @param query Query to get error from.
         * @return true if the query had any error set, or false if not.
         *
         * If the error was defined in the query, its message is stored in lastQueryError.
         */
        bool storeErrorAndReturn(SqlQueryPtr results);
        void printErrorIfSet(SqlQueryPtr results);
        void storeGroup(const DbGroupPtr& group, qint64 parentId = -1);
        void readGroupRecursively(DbGroupPtr group);
        QString getConfigPath();
        void dropTables(const QList<QString>& tables);
        void initTables();
        void initDbFile();
        QList<ConfigDirCandidate> getStdDbPaths();
        bool tryInitDbFile(const ConfigDirCandidate& dbPath);
        QVariant deserializeValue(const QVariant& value) const;

        void asyncAddSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void asyncUpdateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void asyncClearSqlHistory();
        void asyncDeleteSqlHistory(const QList<qint64> &ids);

        void asyncAddCliHistory(const QString& text);
        void asyncApplyCliHistoryLimit();
        void asyncClearCliHistory();

        void asyncAddBindParamHistory(const QVector<QPair<QString, QVariant>>& params);
        void asyncApplyBindParamHistoryLimit();

        void asyncAddPopulateHistory(const QString& database, const QString& table, int rows, const QHash<QString, QPair<QString, QVariant>>& columnsPluginsConfig);
        void asyncApplyPopulateHistoryLimit();

        void asyncAddDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile);
        void asyncClearDdlHistory();

        void asyncAddReportHistory(bool isFeatureRequest, const QString& title, const QString& url);
        void asyncDeleteReport(int id);
        void asyncClearReportHistory();

        void mergeMasterConfig();
        void updateConfigDb();
        bool tryToMigrateOldGlobalPath(const QString& oldPath, const QString& newPath);
        QString getLegacyConfigPath();

        static Config* instance;
        static qint64 sqlHistoryId;
        static QString memoryDbName;

        Db* db = nullptr;
        QString configDir;
        QString lastQueryError;
        bool massSaving = false;
        SqlHistoryModel* sqlHistoryModel = nullptr;
        DdlHistoryModel* ddlHistoryModel = nullptr;
        QMutex sqlHistoryMutex;
        QMutex ddlHistoryMutex;
        QString sqlite3Version;

    public slots:
        void refreshDdlHistory();
        void refreshSqlHistory();
};

#endif // CONFIGIMPL_H
