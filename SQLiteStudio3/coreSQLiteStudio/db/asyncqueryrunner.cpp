#include "db/asyncqueryrunner.h"
#include "db/sqlquery.h"
#include "db/db.h"
#include <QDebug>

AsyncQueryRunner::AsyncQueryRunner(const QString &query, const QVariant& args, Db::Flags flags)
    : query(query), args(args), flags(flags)
{
    init();
}

void AsyncQueryRunner::init()
{
    setAutoDelete(false);
}

void AsyncQueryRunner::run()
{
    if (!db || !db->isValid())
    {
        qCritical() << "No Db or invalid Db defined in AsyncQueryRunner!";
        emit finished(this);
    }

    SqlQueryPtr res;
    if (args.userType() == QMetaType::QVariantList)
    {
        res = db->exec(query, args.toList(), flags);
    }
    else if (args.userType() == QMetaType::QVariantHash)
    {
        res = db->exec(query, args.toHash(), flags);
    }
    else
    {
        qCritical() << "Invalid argument type in AsyncQueryRunner::run():" << args.userType();
    }

    results = SqlQueryPtr(res);
    emit finished(this);
}

SqlQueryPtr AsyncQueryRunner::getResults()
{
    return results;
}


void AsyncQueryRunner::setDb(Db *db)
{
    this->db = db;
}

void AsyncQueryRunner::setAsyncId(quint32 id)
{
    asyncId = id;
}

quint32 AsyncQueryRunner::getAsyncId()
{
    return asyncId;
}
