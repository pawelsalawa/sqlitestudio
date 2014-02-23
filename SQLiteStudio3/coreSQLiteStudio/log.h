#ifndef LOG_H
#define LOG_H

#include "db/db.h"
#include <QString>
#include <QHash>
#include <QList>
#include <QVariant>

void logSql(Db* db, const QString& str, const QHash<QString,QVariant>& args, Db::Flags flags);
void logSql(Db* db, const QString& str, const QList<QVariant>& args, Db::Flags flags);

#endif // LOG_H
