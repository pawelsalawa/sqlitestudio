#ifndef CONFIG_H
#define CONFIG_H

#include "coreSQLiteStudio_global.h"
#include "config_builder.h"
#include "sqlitestudio.h"
#include "common/utils.h"
#include <QObject>
#include <QVariant>
#include <QHash>
#include <QStringList>
#include <QSharedPointer>
#include <QDateTime>

const int SQLITESTUDIO_CONFIG_VERSION = 5;

CFG_CATEGORIES(Core,
    CFG_CATEGORY(General,
        CFG_ENTRY(int,          SqlHistorySize,          10000)
        CFG_ENTRY(int,          DdlHistorySize,          1000)
        CFG_ENTRY(int,          BindParamsCacheSize,     1000)
        CFG_ENTRY(int,          PopulateHistorySize,     100)
        CFG_ENTRY(QString,      LoadedPlugins,           "")
        CFG_ENTRY(QVariantHash, ActiveCodeFormatter,     QVariantHash())
        CFG_ENTRY(bool,         CheckUpdatesOnStartup,   true)
        CFG_ENTRY(QString,      Language,                "en")
    )
    CFG_CATEGORY(Console,
        CFG_ENTRY(int,          HistorySize,             100)
    )
    CFG_CATEGORY(Internal,
        CFG_ENTRY(QVariantList, Collations,              QVariantList())
        CFG_ENTRY(QVariantList, Extensions,              QVariantList())
        CFG_ENTRY(QVariantList, CodeSnippets,            QVariantList())
        CFG_ENTRY(QString,      BugReportUser,           QString())
        CFG_ENTRY(QString,      BugReportPassword,       QString())
        CFG_ENTRY(QString,      BugReportRecentTitle,    QString())
        CFG_ENTRY(QString,      BugReportRecentContents, QString())
        CFG_ENTRY(bool,         BugReportRecentError,    false)
        CFG_ENTRY(bool,         DefaultSnippetsCreated,  false)
    )
    CFG_CATEGORY(CodeAssistant,
        CFG_ENTRY(bool,         AutoTrigger,             true)
    )
)

#define CFG_CORE CFG_INSTANCE(Core)

class QAbstractItemModel;
class DdlHistoryModel;
class QSettings;

class API_EXPORT Config : public QObject
{
    Q_OBJECT

    public:
        virtual ~Config();

        struct CfgDb
        {
            QString name;
            QString path;
            QHash<QString,QVariant> options;
        };

        typedef QSharedPointer<CfgDb> CfgDbPtr;

        struct DbGroup;
        typedef QSharedPointer<DbGroup> DbGroupPtr;

        struct DbGroup
        {
            qint64 id;
            QString referencedDbName;
            QString name;
            QList<DbGroupPtr> childs;
            int order;
            bool open = false;
            bool dbExpanded = false;
        };

        struct SqlHistoryEntry
        {
            QString query;
            QString dbName;
            int rowsAffected;
            int unixtime;
        };

        typedef QSharedPointer<SqlHistoryEntry> SqlHistoryEntryPtr;

        struct DdlHistoryEntry
        {
            QString dbName;
            QString dbFile;
            QDateTime timestamp;
            QString queries;
        };

        typedef QSharedPointer<DdlHistoryEntry> DdlHistoryEntryPtr;

        struct ReportHistoryEntry
        {
            int id = 0;
            bool isFeatureRequest = false;
            int timestamp = 0;
            QString title;
            QString url;
        };

        typedef QSharedPointer<ReportHistoryEntry> ReportHistoryEntryPtr;

        typedef std::function<QString()> AskUserForConfigDirFunc;

        static void setMasterConfigFile(const QString& path);
        static QString getMasterConfigFile();
        static void setAskUserForConfigDirFunc(const AskUserForConfigDirFunc& value);
        static QString getPortableConfigPath();
        static QSettings* getSettings();

        virtual void init() = 0;
        virtual const QString& getConfigDir() const = 0;
        virtual QString getConfigFilePath() const = 0;
        virtual bool isInMemory() const = 0;

        virtual void beginMassSave() = 0;
        virtual void commitMassSave() = 0;
        virtual void rollbackMassSave() = 0;
        virtual bool isMassSaving() const = 0;
        virtual void set(const QString& group, const QString& key, const QVariant& value) = 0;
        virtual QVariant get(const QString& group, const QString& key) = 0;
        virtual QVariant get(const QString& group, const QString& key, const QVariant& defaultValue) = 0;
        virtual QHash<QString,QVariant> getAll() = 0;

        virtual bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant> &options) = 0;
        virtual bool updateDb(const QString& name, const QString &newName, const QString& path, const QHash<QString, QVariant> &options) = 0;
        virtual bool removeDb(const QString& name) = 0;
        virtual bool isDbInConfig(const QString& name) = 0;
        virtual QString getLastErrorString() const = 0;

        /**
         * @brief Provides list of all registered databases.
         * @return List of database entries.
         *
         * Registered databases are those that user added to the application. They are not necessary valid or supported.
         * They can be inexisting or unsupported, but they are kept in registry in case user fixes file path,
         * or loads plugin to support it.
         */
        virtual QList<CfgDbPtr> dbList() = 0;
        virtual CfgDbPtr getDb(const QString& dbName) = 0;

        virtual void storeGroups(const QList<DbGroupPtr>& groups) = 0;
        virtual QList<DbGroupPtr> getGroups() = 0;
        virtual DbGroupPtr getDbGroup(const QString& dbName) = 0;

        virtual qint64 addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected) = 0;
        virtual void updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected) = 0;
        virtual void clearSqlHistory() = 0;
        virtual void deleteSqlHistory(const QList<qint64>& ids) = 0;
        virtual QAbstractItemModel* getSqlHistoryModel() = 0;

        virtual void addCliHistory(const QString& text) = 0;
        virtual void applyCliHistoryLimit() = 0;
        virtual void clearCliHistory() = 0;
        virtual QStringList getCliHistory() const = 0;

        virtual void addBindParamHistory(const QVector<QPair<QString, QVariant>>& params) = 0;
        virtual void applyBindParamHistoryLimit() = 0;
        virtual QVector<QPair<QString, QVariant>> getBindParamHistory(const QStringList& paramNames) const = 0;

        virtual void addPopulateHistory(const QString& database, const QString& table, int rows, const QHash<QString, QPair<QString, QVariant>>& columnsPluginsConfig) = 0;
        virtual void applyPopulateHistoryLimit() = 0;
        virtual QHash<QString, QPair<QString, QVariant>> getPopulateHistory(const QString& database, const QString& table, int& rows) const = 0;
        virtual QVariant getPopulateHistory(const QString& pluginName) const = 0;

        virtual void addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile) = 0;
        virtual QList<DdlHistoryEntryPtr> getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date) = 0;
        virtual DdlHistoryModel* getDdlHistoryModel() = 0;
        virtual void clearDdlHistory() = 0;

        virtual void addReportHistory(bool isFeatureRequest, const QString& title, const QString& url) = 0;
        virtual QList<ReportHistoryEntryPtr> getReportHistory() = 0;
        virtual void deleteReport(int id) = 0;
        virtual void clearReportHistory() = 0;

        virtual QList<QHash<QString, QVariant> > getScriptFunctions() = 0;
        virtual void setScriptFunctions(const QList<QHash<QString, QVariant> >& newFunctions) = 0;

        virtual void begin() = 0;
        virtual void commit() = 0;
        virtual void rollback() = 0;

        virtual QString getSqlite3Version() const = 0;

    signals:
        void massSaveBegins();
        void massSaveCommitted();
        void sqlHistoryRefreshNeeded();
        void ddlHistoryRefreshNeeded();
        void reportsHistoryRefreshNeeded();

    public slots:
        virtual void refreshSqlHistory() = 0;
        virtual void refreshDdlHistory() = 0;

    protected:
        static AskUserForConfigDirFunc askUserForConfigDirFunc;

};

#define CFG SQLITESTUDIO->getConfig()

#endif // CONFIG_H
