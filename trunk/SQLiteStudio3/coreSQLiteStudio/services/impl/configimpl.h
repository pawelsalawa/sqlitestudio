#ifndef CONFIGIMPL_H
#define CONFIGIMPL_H

#include "coreSQLiteStudio_global.h"
#include "services/config.h"
#include "db/sqlquery.h"

class AsyncConfigHandler;
class SqlHistoryModel;

class ConfigImpl : public Config
{
    Q_OBJECT

    friend class AsyncConfigHandler;

    public:
        virtual ~ConfigImpl();

        void init();
        void cleanUp();
        const QString& getConfigDir();

        void beginMassSave();
        void commitMassSave();
        void rollbackMassSave();
        void set(const QString& group, const QString& key, const QVariant& value);
        QVariant get(const QString& group, const QString& key);
        QHash<QString,QVariant> getAll();

        bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant> &options);
        bool updateDb(const QString& name, const QString &newName, const QString& path, const QHash<QString, QVariant> &options);
        bool removeDb(const QString& name);
        bool isDbInConfig(const QString& name);
        QString getLastErrorString() const;

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
        QAbstractItemModel* getSqlHistoryModel();

        void addCliHistory(const QString& text);
        void applyCliHistoryLimit();
        void clearCliHistory();
        QStringList getCliHistory() const;

        void addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile);
        QList<DdlHistoryEntryPtr> getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date);
        DdlHistoryModel* getDdlHistoryModel();
        void clearDdlHistory();

        void begin();
        void commit();
        void rollback();

    private:
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
        QString getPortableConfigPath();
        void initTables();
        void initDbFile();
        bool tryInitDbFile(const QString& dbPath);
        QVariant deserializeValue(const QVariant& value);

        void asyncApplySqlHistoryLimit();
        void asyncUpdateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void asyncClearSqlHistory();

        void asyncAddCliHistory(const QString& text);
        void asyncApplyCliHistoryLimit();
        void asyncClearCliHistory();

        void asyncAddDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile);
        void asyncClearDdlHistory();

        static Config* instance;
        Db* db = nullptr;
        QString configDir;
        QString lastQueryError;
        SqlHistoryModel* sqlHistoryModel = nullptr;
        DdlHistoryModel* ddlHistoryModel = nullptr;

    signals:
        void sqlHistoryRefreshNeeded();
        void ddlHistoryRefreshNeeded();

    private slots:
        void refreshSqlHistory();
        void refreshDdlHistory();
};

#endif // CONFIGIMPL_H
