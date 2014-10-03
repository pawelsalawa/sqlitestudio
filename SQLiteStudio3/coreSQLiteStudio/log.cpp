#include "log.h"
#include <QTime>
#include <QDebug>

bool SQL_DEBUG = false;
QString SQL_DEBUG_FILTER = "";

void setSqlLoggingEnabled(bool enabled)
{
    SQL_DEBUG = enabled;
}

void setSqlLoggingFilter(const QString& filter)
{
    SQL_DEBUG_FILTER = filter;
}

void logSql(Db* db, const QString& str, const QHash<QString,QVariant>& args, Db::Flags flags)
{
    if (!SQL_DEBUG)
        return;

    if (!SQL_DEBUG_FILTER.isEmpty() && SQL_DEBUG_FILTER != db->getName())
        return;

    qDebug() << QString("SQL %1> %2").arg(db->getName()).arg(str) << "(flags:" << Db::flagsToString(flags) << ")";
    QHashIterator<QString,QVariant> it(args);
    while (it.hasNext())
    {
        it.next();
        qDebug() << "    SQL arg>" << it.key() << "=" << it.value();
    }
}

void logSql(Db* db, const QString& str, const QList<QVariant>& args, Db::Flags flags)
{
    if (!SQL_DEBUG)
        return;

    if (!SQL_DEBUG_FILTER.isEmpty() && SQL_DEBUG_FILTER != db->getName())
        return;

    qDebug() << QString("SQL %1> %2").arg(db->getName()).arg(str) << "(flags:" << Db::flagsToString(flags) << ")";
    int i = 0;
    foreach (const QVariant& arg, args)
        qDebug() << "    SQL arg>" << i++ << "=" << arg;
}
