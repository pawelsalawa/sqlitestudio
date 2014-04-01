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
        void set(const QString&, const QString&, const QVariant&);
        QVariant get(const QString&, const QString&);
        QHash<QString, QVariant> getAll();
        bool addDb(const QString&, const QString&, const QHash<QString, QVariant>&);
        bool updateDb(const QString&, const QString&, const QString&, const QHash<QString, QVariant>&);
        bool removeDb(const QString&);
        bool isDbInConfig(const QString&);
        QString getLastErrorString() const;
        QList<CfgDbPtr> dbList();
        CfgDbPtr getDb(const QString&);
        void storeGroups(const QList<DbGroupPtr>&);
        QList<DbGroupPtr> getGroups();
        DbGroupPtr getDbGroup(const QString&);
        qint64 addSqlHistory(const QString&, const QString&, int, int);
        void updateSqlHistory(qint64, const QString&, const QString&, int, int);
        void clearSqlHistory();
        QAbstractItemModel*getSqlHistoryModel();
        void addCliHistory(const QString&);
        void applyCliHistoryLimit();
        void clearCliHistory();
        QStringList getCliHistory() const;
        void addDdlHistory(const QString&, const QString&, const QString&);
        QList<DdlHistoryEntryPtr> getDdlHistoryFor(const QString&, const QString&, const QDate&);
        DdlHistoryModel* getDdlHistoryModel();
        void clearDdlHistory();
        bool setFunctions(const QList<FunctionManager::FunctionPtr>&);
        QList<FunctionManager::FunctionPtr> getFunctions() const;
        void begin();
        void commit();
        void rollback();
        bool setCollations(const QList<CollationManager::CollationPtr>&);
        QList<CollationManager::CollationPtr> getCollations() const;
};

#endif // CONFIGMOCK_H
