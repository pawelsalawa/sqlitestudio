#include "dbsqlite3mock.h"
#include "common/unused.h"
#include <QDebug>

DbSqlite3Mock::DbSqlite3Mock(const QString &name, const QString &path, const QHash<QString,QVariant> &options)
    : DbSqlite3(name, path, options)
{
}
