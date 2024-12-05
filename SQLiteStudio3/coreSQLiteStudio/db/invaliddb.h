#ifndef INVALIDDB_H
#define INVALIDDB_H

#include "db/db.h"

class API_EXPORT InvalidDb : public Db
{
    public:
        InvalidDb(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        bool isOpen();
        QString getName() const;
        QString getPath() const;
        quint8 getVersion() const;
        QString getEncoding();
        QHash<QString, QVariant>& getConnectionOptions();
        void setName(const QString& value);
        void setPath(const QString& value);
        void setConnectionOptions(const QHash<QString, QVariant>& value);
        void setTimeout(int secs);
        int getTimeout() const;
        QList<AliasedColumn> columnsForQuery(const QString& query);
        SqlQueryPtr exec(const QString& query, const QList<QVariant>& args, Flags flags);
        SqlQueryPtr exec(const QString& query, const QHash<QString, QVariant>& args, Flags flags);
        SqlQueryPtr exec(const QString& query, Db::Flags flags);
        SqlQueryPtr exec(const QString& query, const QVariant& arg);
        SqlQueryPtr exec(const QString& query, std::initializer_list<QVariant> argList);
        SqlQueryPtr exec(const QString& query, std::initializer_list<std::pair<QString, QVariant> > argMap);
        void asyncExec(const QString& query, const QList<QVariant>& args, QueryResultsHandler resultsHandler, Flags flags);
        void asyncExec(const QString& query, const QHash<QString, QVariant>& args, QueryResultsHandler resultsHandler, Flags flags);
        void asyncExec(const QString& query, QueryResultsHandler resultsHandler, Flags flags);
        quint32 asyncExec(const QString& query, const QList<QVariant>& args, Flags flags);
        quint32 asyncExec(const QString& query, const QHash<QString, QVariant>& args, Flags flags);
        quint32 asyncExec(const QString& query, Flags flags);
        SqlQueryPtr prepare(const QString& query);
        bool begin(bool noLock = false);
        bool commit(bool noLock = false);
        bool rollback(bool noLock = false);
        void asyncInterrupt();
        bool isReadable();
        bool isWritable();
        QString attach(Db* otherDb, bool silent);
        AttachGuard guardedAttach(Db* otherDb, bool silent);
        void detach(Db* otherDb);
        void detachAll();
        const QHash<Db*, QString>& getAttachedDatabases();
        QSet<QString> getAllAttaches();
        QString getUniqueNewObjectName(const QString& attachedDbName);
        QString getErrorText();
        int getErrorCode();
        QString getTypeLabel() const;
        QString getTypeClassName() const;
        bool initAfterCreated();
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount, bool deterministic);
        bool registerAggregateFunction(const QString& name, int argCount, bool deterministic);
        bool registerCollation(const QString& name);
        bool deregisterCollation(const QString& name);
        void interrupt();
        bool isValid() const;
        QString getError() const;
        void setError(const QString& value);
        bool loadExtension(const QString& filePath, const QString& initFunc);
        bool isComplete(const QString& sql) const;
        Db* clone() const;
        bool isTransactionActive() const;

    public slots:
        bool open();
        bool close();
        bool openQuiet();
        bool openForProbing();
        bool closeQuiet();
        void registerUserFunctions();
        void registerUserCollations();

    private:
        QString name;
        QString path;
        QHash<QString, QVariant> connOptions;
        int timeout = 0;
        QHash<Db*, QString> attachedDbs;
        QString error;
};

#endif // INVALIDDB_H
