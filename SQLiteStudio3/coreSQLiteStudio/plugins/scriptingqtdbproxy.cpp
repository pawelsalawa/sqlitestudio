#include "scriptingqt.h"
#include "scriptingqtdbproxy.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include <QJSEngine>

ScriptingQtDbProxy::ScriptingQtDbProxy(QJSEngine* engine, QObject *parent) :
    QObject(parent), engine(engine)
{
}
Db* ScriptingQtDbProxy::getDb() const
{
    return db;
}

void ScriptingQtDbProxy::setDb(Db* value)
{
    db = value;
}
bool ScriptingQtDbProxy::getUseDbLocking() const
{
    return useDbLocking;
}

void ScriptingQtDbProxy::setUseDbLocking(bool value)
{
    useDbLocking = value;
}

QHash<QString, QVariant> ScriptingQtDbProxy::mapToHash(const QMap<QString, QVariant>& map)
{
    QHash<QString, QVariant> hash;
    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext())
    {
        it.next();
        hash[it.key()] = it.value();
    }
    return hash;
}

QVariant ScriptingQtDbProxy::evalInternal(const QString& sql, const QList<QVariant>& listArgs, const QMap<QString, QVariant>& mapArgs,
                                          bool singleCell, const QJSValue* funcPtr)
{
    if (!db)
    {
        QString funcName = singleCell ? QStringLiteral("db.onecolumn()") : QStringLiteral("db.eval()");
        engine->throwError(tr("No database available in current context, while called JavaScript's %1 command.").arg(funcName));
        return evalInternalErrorResult(singleCell);
    }

    Db::Flags flags;
    if (!useDbLocking)
        flags |= Db::Flag::NO_LOCK;

    SqlQueryPtr results;
    if (listArgs.size() > 0)
        results = db->exec(sql, listArgs, flags);
    else
        results = db->exec(sql, mapToHash(mapArgs), flags);

    if (results->isError())
    {
        QString funcName = singleCell ? QStringLiteral("db.onecolumn()") : QStringLiteral("db.eval()");
        engine->throwError(tr("Error from %1: %2").arg(funcName, results->getErrorText()));
        return evalInternalErrorResult(singleCell);
    }

    if (singleCell)
    {
        return results->getSingleCell();
    }
    else if (funcPtr)
    {
        QJSValue func(*funcPtr);
        SqlResultsRowPtr row;
        QJSValueList funcArgs;
        QJSValue funcResult;
        while (results->hasNext())
        {
            row = results->next();
            funcArgs = ScriptingQt::toValueList(engine, row->valueList());
            funcResult = func.call(funcArgs);
            if (!funcResult.isUndefined())
                break;
        }
        return funcResult.toVariant();
    }
    else
    {
        QList<QVariant> evalResults;
        SqlResultsRowPtr row;
        while (results->hasNext())
        {
            row = results->next();
            evalResults << QVariant(row->valueList());
        }
        return evalResults;
    }
}

QVariant ScriptingQtDbProxy::evalInternalErrorResult(bool singleCell)
{
    QList<QVariant> result;
    if (singleCell)
        result << QVariant();

    return result;
}

QVariant ScriptingQtDbProxy::eval(const QString& sql)
{
    return evalInternal(sql, QList<QVariant>(), QMap<QString, QVariant>(), false);
}

QVariant ScriptingQtDbProxy::eval(const QString& sql, const QList<QVariant>& args)
{
    return evalInternal(sql, args, QMap<QString, QVariant>(), false);
}

QVariant ScriptingQtDbProxy::eval(const QString& sql, const QMap<QString, QVariant>& args)
{
    return evalInternal(sql, QList<QVariant>(), args, false);
}

QVariant ScriptingQtDbProxy::eval(const QString& sql, const QList<QVariant>& args, const QJSValue& func)
{
    return evalInternal(sql, args, QMap<QString, QVariant>(), false, &func);
}

QVariant ScriptingQtDbProxy::eval(const QString& sql, const QMap<QString, QVariant>& args, const QJSValue& func)
{
    return evalInternal(sql, QList<QVariant>(), args, false, &func);
}

QVariant ScriptingQtDbProxy::onecolumn(const QString& sql, const QList<QVariant>& args)
{
    return evalInternal(sql, args, QMap<QString, QVariant>(), true);
}

QVariant ScriptingQtDbProxy::onecolumn(const QString& sql, const QMap<QString, QVariant>& args)
{
    return evalInternal(sql, QList<QVariant>(), args, true);
}

