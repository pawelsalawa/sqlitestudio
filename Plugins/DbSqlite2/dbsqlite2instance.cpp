#include "dbsqlite2instance.h"

DbSqlite2Instance::DbSqlite2Instance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb2<Sqlite2>(name, path, connOptions)
{

}

QString DbSqlite2Instance::getEncoding()
{
    return "UTF-8";
}
