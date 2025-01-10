#include "scriptingsql.h"
#include "common/unused.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include "services/dbmanager.h"
#include "common/utils_sql.h"

ScriptingSql::ScriptingSql()
{
}

ScriptingSql::~ScriptingSql()
{
}

QString ScriptingSql::getLanguage() const
{
    return "SQL";
}

ScriptingPlugin::Context* ScriptingSql::createContext()
{
    SqlContext* ctx = new SqlContext();
    contexts << ctx;
    return ctx;
}

void ScriptingSql::releaseContext(ScriptingPlugin::Context* context)
{
    if (!contexts.contains(context))
        return;

    delete context;
    contexts.removeOne(context);
}

void ScriptingSql::resetContext(ScriptingPlugin::Context* context)
{
    dynamic_cast<SqlContext*>(context)->errorText.clear();
}

QVariant ScriptingSql::evaluate(ScriptingPlugin::Context* context, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking)
{
    SqlContext* ctx = dynamic_cast<SqlContext*>(context);
    ctx->errorText.clear();

    Db* theDb = nullptr;
    if (db && db->isValid())
        theDb = db;
    else if (memDb)
        theDb = memDb;
    else
        return QVariant();

    Db::Flags execFlags;
    if (!locking)
        execFlags |= Db::Flag::NO_LOCK;

    QString sql = code;
    if (ctx->variables.size() > 0)
    {
        QList<QString> keys = ctx->variables.keys();
        std::sort(keys.begin(), keys.end(), std::greater<QString>());
        for (const QString& key : keys)
        {
            QString value = "'" + ctx->variables[key].toString() + "'";
            sql.replace(":" + key, value).replace("@" + key, value).replace("$" + key, value);
        }
    }

    replaceNamedArgs(sql, funcInfo, args);

    SqlQueryPtr result = theDb->exec(sql, args, execFlags);
    if (result->isError())
    {
        dynamic_cast<SqlContext*>(context)->errorText = result->getErrorText();
        return QVariant();
    }

    return result->getSingleCell();
}

QVariant ScriptingSql::evaluate(const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
{
    Db* theDb = nullptr;

    if (db && db->isValid())
        theDb = db;
    else if (memDb)
        theDb = memDb;
    else
        return QVariant();

    Db::Flags execFlags;
    if (!locking)
        execFlags |= Db::Flag::NO_LOCK;

    QString sql = code;
    replaceNamedArgs(sql, funcInfo, args);

    SqlQueryPtr result = theDb->exec(sql, args, execFlags);
    if (result->isError())
    {
        *errorMessage = result->getErrorText();
        return QVariant();
    }

    return result->getSingleCell();
}

void ScriptingSql::setVariable(ScriptingPlugin::Context* context, const QString& name, const QVariant& value)
{
    dynamic_cast<SqlContext*>(context)->variables[name] = value;
}

QVariant ScriptingSql::getVariable(ScriptingPlugin::Context* context, const QString& name)
{
    if (dynamic_cast<SqlContext*>(context)->variables.contains(name))
        return dynamic_cast<SqlContext*>(context)->variables[name];

    return QVariant();
}

bool ScriptingSql::hasError(ScriptingPlugin::Context* context) const
{
    return !getErrorMessage(context).isNull();
}

QString ScriptingSql::getErrorMessage(ScriptingPlugin::Context* context) const
{
    return dynamic_cast<SqlContext*>(context)->errorText;
}

QString ScriptingSql::getIconPath() const
{
    return ":/images/plugins/scriptingsql.svg";
}

bool ScriptingSql::init()
{
    memDb = DBLIST->createInMemDb();
    return memDb != nullptr;
}

void ScriptingSql::deinit()
{
    for (Context* context : contexts)
        delete context;

    contexts.clear();

    safe_delete(memDb);
}

void ScriptingSql::replaceNamedArgs(QString& sql, const ScriptingPlugin::FunctionInfo& funcInfo, const QList<QVariant>& args)
{
    // First build map of argName to its value in order in which arguments were passed to the function
    int i = 0;
    QStringList argNames = funcInfo.getArguments();
    QHash<QString, QString> argMap;
    for (const QString& argName : argNames)
    {
        if (i >= args.size())
            break;

        argMap[argName] = valueToSqlLiteral(args[i++]);
    }

    // Then sort arguments in alphabetically descending order, to prevent replacing shorter names first
    // and proceed with argument substitutions
    std::sort(argNames.begin(), argNames.end(), std::greater<QString>());
    for (const QString& argName : argNames)
    {
        QString value = argMap[argName];
        sql.replace(":" + argName, value)
           .replace("@" + argName, value)
           .replace("$" + argName, value);
    }
}
