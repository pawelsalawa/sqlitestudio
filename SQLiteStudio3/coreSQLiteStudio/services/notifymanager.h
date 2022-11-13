#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include "db/db.h"
#include "common/global.h"
#include <QStringList>
#include <QObject>

class API_EXPORT NotifyManager : public QObject
{
    Q_OBJECT

    DECLARE_SINGLETON(NotifyManager)

    public:
        explicit NotifyManager(QObject *parent = 0);

        QList<QString> getRecentErrors() const;
        QList<QString> getRecentWarnings() const;
        QList<QString> getRecentInfos() const;

    signals:
        void notifyError(const QString& msg);
        void notifyWarning(const QString& msg);
        void notifyInfo(const QString& msg);

        void objectModified(Db* db, const QString& database, const QString& object);
        void objectDeleted(Db* db, const QString& database, const QString& object);
        void objectCreated(Db* db, const QString& database, const QString& object);
        void objectRenamed(Db* db, const QString& database, const QString& oldObject, const QString& newObject);

    public slots:
        void error(const QString& msg);
        void warn(const QString& msg);
        void info(const QString& msg);

        void modified(Db* db, const QString& database, const QString& object);
        void deleted(Db* db, const QString& database, const QString& object);
        void created(Db* db, const QString& database, const QString& object);
        void renamed(Db* db, const QString& database, const QString& oldObject, const QString& newObject);

    private:
        void addToRecentList(QStringList& list, const QString& message);

        static const constexpr int maxRecentMessages = 10;

        QStringList recentErrors;
        QStringList recentWarnings;
        QStringList recentInfos;
};

#define NOTIFY_MANAGER NotifyManager::getInstance()

void API_EXPORT notifyError(const QString& msg);
void API_EXPORT notifyWarn(const QString& msg);
void API_EXPORT notifyInfo(const QString& msg);

#endif // NOTIFYMANAGER_H
