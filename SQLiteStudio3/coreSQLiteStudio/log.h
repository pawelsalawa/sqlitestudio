#ifndef LOG_H
#define LOG_H

#include "db/db.h"
#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QHash>
#include <QList>
#include <QVariant>

API_EXPORT void logSql(Db* db, const QString& str, const QHash<QString,QVariant>& args, Db::Flags flags);
API_EXPORT void logSql(Db* db, const QString& str, const QList<QVariant>& args, Db::Flags flags);
API_EXPORT void setSqlLoggingEnabled(bool enabled);
API_EXPORT void setSqlLoggingFilter(const QString& filter);

#endif // LOG_H
