#ifndef CONFIGMOCK_H
#define CONFIGMOCK_H

#include "services/config.h"

#include <QAbstractItemModel>

class ConfigMock : public Config
{
    public:
        void init();
        void cleanUp();
        const QString& getConfigDir();
        void beginMassSave();
        void commitMassSave();
        void rollbackMassSave();
        void set(const QString& group, const QString& key, const QVariant& value);
        QVariant get(const QString& group, const QString& key);
        QHash<QString, QVariant> getAll();
        bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant>& options);
        bool updateDb(const QString& name, const QString& newName, const QString& path, const QHash<QString, QVariant>& options);
        bool removeDb(const QString& name);
        bool isDbInConfig(const QString& name);
        QString getLastErrorString() const;
        QList<CfgDbPtr> dbList();
        CfgDbPtr getDb(const QString& dbName);
        void storeGroups(const QList<DbGroupPtr>& groups);
        QList<DbGroupPtr> getGroups();
        DbGroupPtr getDbGroup(const QString& dbName);
        qint64 addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected);
        void clearSqlHistory();
        QAbstractItemModel*getSqlHistoryModel();
        void addCliHistory(const QString& text);
        void applyCliHistoryLimit();
        void clearCliHistory();
        QStringList getCliHistory() const;
        void addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile);
        QList<DdlHistoryEntryPtr> getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date);
        DdlHistoryModel* getDdlHistoryModel();
        void clearDdlHistory();
        bool setFunctions(const QList<FunctionManager::FunctionPtr>& functions);
        QList<FunctionManager::FunctionPtr> getFunctions() const;
        void begin();
        void commit();
        void rollback();
};

#endif // CONFIGMOCK_H
