#include "scriptingsql.h"
#include "common/unused.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include "services/dbmanager.h"

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

QVariant ScriptingSql::evaluate(ScriptingPlugin::Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking)
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
        QString value;
        for (const QString& key : ctx->variables.keys())
        {
            value = "'" + ctx->variables[key].toString() + "'";
            sql.replace(":" + key, value).replace("@" + key, value).replace("$" + key, value);
        }
    }

    SqlQueryPtr result = theDb->exec(sql, args, execFlags);
    if (result->isError())
    {
        dynamic_cast<SqlContext*>(context)->errorText = result->getErrorText();
        return QVariant();
    }

    return result->getSingleCell();
}

QVariant ScriptingSql::evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
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

    SqlQueryPtr result = theDb->exec(code, args, execFlags);
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

QByteArray ScriptingSql::getIconData() const
{
    static_char* icon = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJ"
            "bWFnZVJlYWR5ccllPAAAARpJREFUeNrE0z9LQlEYx/Fr/gXdJILegSQJEk13yrUxxDFwUhqagyAE"
            "F8E34BsIFKlozpqao8K9WVrCJSWu30d+F44SOjj4wId77nPOeTjn3HMjQRB4m8SOt2Fsv0CE8Hle"
            "YoRdnGEPF8jhB4+4xw0ONLc7R4E7VJBX23JNtJBFSflD9YXjrT3fQhxpfKqqRQEf+MaLxuTVF3e3"
            "bo02yjjBrTMojD+9R52+sO3F8KxC13YmeELCHbT0vtBnE6ua1EfDGZRQO7qiQNEK1LiNRzynSKpj"
            "CJ+8fY1T5d/VZ+2Y5nQs8YVXPW01+zjGg3JvuFK+q1yoZ3tOaVnJpcOzqOtwzzF28rbaX0zWXbQM"
            "Bvqc/r83cet/40yAAQCHjz1eQkhXqAAAAABJRU5ErkJggg==";
    return QByteArray(icon);
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
