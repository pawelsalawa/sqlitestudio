#ifndef CONFIGMOCK_H
#define CONFIGMOCK_H

#include "services/config.h"

#include <QAbstractItemModel>

class ConfigMock : public Config
{
    public:
        void init();
        const QString& getConfigDir();
        void beginMassSave();
        void commitMassSave();
        void rollbackMassSave();
        void set(const QString&, const QString&, const QVariant&);
        QVariant get(const QString&, const QString&);
        QVariant get(const QString&, const QString&, const QVariant&);
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
        void deleteSqlHistory(const QList<qint64>&);
        QAbstractItemModel*getSqlHistoryModel();
        void addCliHistory(const QString&);
        void applyCliHistoryLimit();
        void clearCliHistory();
        QStringList getCliHistory() const;
        void addBindParamHistory(const QVector<QPair<QString, QVariant>>&);
        void applyBindParamHistoryLimit();
        QVector<QPair<QString, QVariant>> getBindParamHistory(const QStringList&) const;
        void addPopulateHistory(const QString&, const QString&, int, const QHash<QString, QPair<QString, QVariant>>&);
        void applyPopulateHistoryLimit();
        QHash<QString, QPair<QString, QVariant>> getPopulateHistory(const QString&, const QString&, int&) const;
        QVariant getPopulateHistory(const QString&) const;
        void addDdlHistory(const QString&, const QString&, const QString&);
        QList<DdlHistoryEntryPtr> getDdlHistoryFor(const QString&, const QString&, const QDate&);
        DdlHistoryModel* getDdlHistoryModel();
        void clearDdlHistory();
        QList<QHash<QString, QVariant> > getScriptFunctions();
        void setScriptFunctions(const QList<QHash<QString, QVariant> >&);
        void begin();
        void commit();
        void rollback();
        const QString &getConfigDir() const;
        QString getConfigFilePath() const;
        bool isMassSaving() const;
        void addReportHistory(bool, const QString &, const QString &);
        QList<ReportHistoryEntryPtr> getReportHistory();
        void deleteReport(int);
        void clearReportHistory();
        QString getSqlite3Version() const;
        bool isInMemory() const;

    public slots:
        void refreshSqlHistory();
        void refreshDdlHistory();
};

#endif // CONFIGMOCK_H
