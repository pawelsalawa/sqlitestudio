#ifndef DBANDROIDINSTANCE_H
#define DBANDROIDINSTANCE_H

#include "db/abstractdb.h"
#include <QObject>
#include <functional>
#include <QCache>

class DbAndroidConnection;
class DbAndroid;

class DbAndroidInstance : public AbstractDb
{
        Q_OBJECT

    public:
        typedef std::function<void(const QStringList&)> AsyncDbListResponseHandler;

        DbAndroidInstance(DbAndroid* plugin, const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);
        ~DbAndroidInstance();

        QList<AliasedColumn> columnsForQuery(const QString& query);
        SqlQueryPtr prepare(const QString& query);
        QString getTypeLabel() const;
        QString getTypeClassName() const;
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount, bool deterministic);
        bool registerAggregateFunction(const QString& name, int argCount, bool deterministic);
        bool initAfterCreated();
        bool loadExtension(const QString& filePath, const QString& initFunc);
        bool isComplete(const QString& sql) const;
        Db* clone() const;
        bool isTransactionActive() const;

    protected:
        bool isOpenInternal();
        void interruptExecution();
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        bool flushWalInternal();
        bool registerCollationInternal(const QString& name);
        bool deregisterCollationInternal(const QString& name);

    private:
        DbAndroidConnection* createConnection();

        DbAndroid* plugin = nullptr;
        DbAndroidConnection* connection = nullptr;
        int errorCode = 0;
        QString errorText;

    private slots:
        void handleDisconnected();
};

#endif // DBANDROIDINSTANCE_H
