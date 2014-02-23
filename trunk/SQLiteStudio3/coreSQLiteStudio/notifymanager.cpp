#include "notifymanager.h"

NotifyManager::NotifyManager(QObject *parent) :
    QObject(parent)
{
}

void NotifyManager::error(const QString &msg)
{
    addToRecentList(recentErrors, msg);
    emit notifyError(msg);
}

void NotifyManager::warn(const QString &msg)
{
    addToRecentList(recentWarnings, msg);
    emit notifyWarning(msg);
}

void NotifyManager::info(const QString &msg)
{
    addToRecentList(recentInfos, msg);
    emit notifyInfo(msg);
}

void NotifyManager::modified(Db* db, const QString& database, const QString& object)
{
    emit objectModified(db, database, object);
}

void NotifyManager::deleted(Db* db, const QString& database, const QString& object)
{
    emit objectDeleted(db, database, object);
}

void NotifyManager::createded(Db* db, const QString& database, const QString& object)
{
    emit objectCreated(db, database, object);
}

void NotifyManager::renamed(Db* db, const QString& database, const QString& oldObject, const QString& newObject)
{
    emit objectRenamed(db, database, oldObject, newObject);
}

void NotifyManager::addToRecentList(QStringList& list, const QString &message)
{
    list << message;
    if (list.size() <= maxRecentMessages)
        return;

    list = list.mid(list.length() - maxRecentMessages);
}

QList<QString> NotifyManager::getRecentInfos() const
{
    return recentInfos;
}

QList<QString> NotifyManager::getRecentWarnings() const
{
    return recentWarnings;
}

QList<QString> NotifyManager::getRecentErrors() const
{
    return recentErrors;
}

void notifyError(const QString &msg)
{
    SQLiteStudio::getInstance()->getNotifyManager()->error(msg);
}

void notifyWarn(const QString &msg)
{
    SQLiteStudio::getInstance()->getNotifyManager()->warn(msg);
}

void notifyInfo(const QString &msg)
{
    SQLiteStudio::getInstance()->getNotifyManager()->info(msg);
}
