#ifndef CONFIG_H
#define CONFIG_H

#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include "cfginternals.h"
#include "services/functionmanager.h"
#include <QObject>
#include <QVariant>
#include <QHash>
#include <QStringList>
#include <QSharedPointer>
#include <QDateTime>

const int SQLITESTUDIO_CONFIG_VERSION = 1;

CFG_CATEGORIES(Core,
    CFG_CATEGORY(General,
        CFG_ENTRY(int,     SqlHistorySize,     10000)
        CFG_ENTRY(int,     DdlHistorySize,     1000)
        CFG_ENTRY(QString, LoadedPlugins,      "")
        CFG_ENTRY(QString, ActiveSqlFormatter, QString())
    )
    CFG_CATEGORY(Console,
        CFG_ENTRY(int,     HistorySize,        100)
    )
)

CFG_DECLARE(Core)
#define CFG_CORE CFG_INSTANCE(Core)

class QSqlDatabase;
class QAbstractItemModel;
class DdlHistoryModel;
class QSqlQuery;

class API_EXPORT Config : public QObject
{
    Q_OBJECT

    DECLARE_SINGLETON(Config)

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

        bool setFunctions(const QList<FunctionManager::FunctionPtr>& functions);
        QList<FunctionManager::FunctionPtr> getFunctions() const;

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
        bool storeErrorAndReturn(const QSqlQuery& query);
        void printErrorIfSet(const QSqlQuery& query);
        void storeGroup(const DbGroupPtr& group, const QVariant &parentId);
        void readGroupRecursively(DbGroupPtr group);
        QString getConfigPath();
        QString getPortableConfigPath();
        void initTables();
        void initDbFile();
        bool tryInitDbFile(const QString& dbPath);
        QVariant deserializeValue(const QVariant& value);

        static Config* instance;
        QSqlDatabase *db = nullptr;
        QString configDir;
        QString lastQueryError;
        QAbstractItemModel* sqlHistoryModel = nullptr;
        DdlHistoryModel* ddlHistoryModel = nullptr;
};

#define CFG Config::getInstance()

#endif // CONFIG_H
