#include "dbsqlite3.h"

DbSqlite3::DbSqlite3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3(name, path, connOptions)
{
}

DbSqlite3::DbSqlite3(const QString& name, const QString& path) :
    DbSqlite3(name, path, QHash<QString,QVariant>())
{
}

bool DbSqlite3::complete(const QString& sql)
{
    return Sqlite3::complete(sql.toUtf8().constData());
}

bool DbSqlite3::isDbFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.read(16);
    file.close();
    if (data.size() < 16)
        return false;

    return data == QByteArrayLiteral("SQLite format 3\000");
}

Db* DbSqlite3::clone() const
{
    return new DbSqlite3(name, path, connOptions);
}

QString DbSqlite3::getTypeClassName() const
{
    return "DbSqlite3";
}
