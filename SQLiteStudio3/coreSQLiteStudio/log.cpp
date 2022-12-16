#include "log.h"
#include "db/queryexecutorsteps/queryexecutorstep.h"
#include <QTime>
#include <QDateTime>
#include <QDebug>

static bool SQL_DEBUG = false;
static bool EXECUTOR_DEBUG = false;
static QString SQL_DEBUG_FILTER = "";

void setSqlLoggingEnabled(bool enabled)
{
    SQL_DEBUG = enabled;
}

void setSqlLoggingFilter(const QString& filter)
{
    SQL_DEBUG_FILTER = filter;
}

QString getLogDateTime()
{
    return QDateTime::currentDateTime().toString("[HH:mm:ss.zzz]");
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
    for (const QVariant& arg : args)
        qDebug() << "    SQL arg>" << i++ << "=" << arg;
}

void setExecutorLoggingEnabled(bool enabled)
{
    EXECUTOR_DEBUG = enabled;
}

void logExecutorStep(QueryExecutorStep* step)
{
    if (!EXECUTOR_DEBUG)
        return;

    qDebug() << getLogDateTime() << "Executing step:" << step->metaObject()->className() << step->objectName();
}


void logExecutorAfterStep(const QString& str)
{
    if (!EXECUTOR_DEBUG)
        return;

    qDebug() << getLogDateTime() << str;
}

bool isExecutorLoggingEnabled()
{
    return EXECUTOR_DEBUG;
}
