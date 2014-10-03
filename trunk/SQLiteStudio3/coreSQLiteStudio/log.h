#ifndef LOG_H
#define LOG_H

#include "db/db.h"
#include "guiSQLiteStudio_global.h"
#include <QString>
#include <QHash>
#include <QList>
#include <QVariant>

GUI_API_EXPORT void logSql(Db* db, const QString& str, const QHash<QString,QVariant>& args, Db::Flags flags);
GUI_API_EXPORT void logSql(Db* db, const QString& str, const QList<QVariant>& args, Db::Flags flags);
GUI_API_EXPORT void setSqlLoggingEnabled(bool enabled);
GUI_API_EXPORT void setSqlLoggingFilter(const QString& filter);

#endif // LOG_H
