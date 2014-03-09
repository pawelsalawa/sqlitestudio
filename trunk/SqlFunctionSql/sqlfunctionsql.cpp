#include "sqlfunctionsql.h"
#include "db/db.h"
#include "unused.h"
#include "pluginmanager.h"
#include "utils.h"
#include "utils_sql.h"
#include <QDir>
#include <parser/lexer.h>

QVariant SqlFunctionSql::evaluateScalar(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, bool& success)
{
    UNUSED(function);

    SqlResultsPtr results;
    QStringList queries = splitQueries(code, db->getDialect());
    QList<QVariant> allArgs = args;
    QList<QVariant> argsForQuery;
    foreach (const QString& query, queries)
    {
        argsForQuery = getArgsForQuery(db, query, allArgs);
        allArgs.mid(argsForQuery.size());

        results = db->exec(query, argsForQuery, Db::Flag::NO_LOCK);
        if (results->isError())
        {
            success = false;
            return results->getErrorText();
        }
    }

    success = true;
    return results->getSingleCell();
}

void SqlFunctionSql::evaluateAggregateInitial(Db* db, const QString& function, int argCount, const QString& code, QHash<QString,QVariant>& aggregateStorage)
{
    aggregateStorage["error"] = false;

    // Find out the unique name for temp table
    QStringList existingNames;
    SqlResultsPtr results = db->exec("SELECT name FROM sqlite_temp_master", Db::Flag::NO_LOCK);
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        existingNames << row->value(0).toString();
    }
    aggregateStorage["table"] = generateUniqueName(function+"_temp_table", existingNames);

    // Create temporary table for aggregate function
    QString createTempTable = "CREATE TEMP TABLE %1 (%2)";
    QStringList cols;
    for (int i = 1; i <= argCount; i++)
        cols << ("arg"+QString::number(i));

    results = db->exec(createTempTable, {aggregateStorage["table"].toString(), cols.join(", ")}, Db::Flag::STRING_REPLACE_ARGS|Db::Flag::NO_LOCK);
    if (results->isError())
    {
        aggregateStorage["error"] = true;
        aggregateStorage["errorText"] = results->getErrorText();
        return;
    }

    // Prepare execution SQL and execute it
    if (code.trimmed().isEmpty())
        return;

    QString preparedCode = code;
    preparedCode.replace("$TABLE", aggregateStorage["table"].toString()).replace("$ARGC", QString::number(argCount));

    QStringList queries = splitQueries(preparedCode, db->getDialect());
    foreach (const QString& query, queries)
    {
        results = db->exec(query, Db::Flag::NO_LOCK);

        // Handle errors - to be reported in the final step
        if (results->isError())
        {
            aggregateStorage["error"] = true;
            aggregateStorage["errorText"] = results->getErrorText();
            break;
        }
    }
}

void SqlFunctionSql::evaluateAggregateStep(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, QHash<QString,QVariant>& aggregateStorage)
{
    UNUSED(function);

    // Check for previous errors
    if (aggregateStorage["error"].toBool())
        return; // initial step failed, no aggregation will be made

    // Prepare middle-step code and execute it
    QString preparedCode = code;
    preparedCode.replace("$TABLE", aggregateStorage["table"].toString()).replace("$ARGC", QString::number(args.size()));

    QList<QVariant> allArgs = args;
    QList<QVariant> argsForQuery;
    QStringList queries = splitQueries(preparedCode, db->getDialect());
    SqlResultsPtr results;
    foreach (const QString& query, queries)
    {
        argsForQuery = getArgsForQuery(db, query, allArgs);
        allArgs.mid(argsForQuery.size());

        results = db->exec(query, argsForQuery, Db::Flag::NO_LOCK);
        if (results->isError())
        {
            aggregateStorage["error"] = true;
            aggregateStorage["errorText"] = results->getErrorText();
            break;
        }
    }
}

QVariant SqlFunctionSql::evaluateAggregateFinal(Db* db, const QString& function, int argCount, const QString& code, bool& success, QHash<QString,QVariant>& aggregateStorage)
{
    UNUSED(function);

    // Check previous errors
    if (aggregateStorage["error"].toBool())
    {
        // Initial or middle step failed, no aggregation will be made
        success = false;
        return aggregateStorage["errorText"].toString();
    }

    // Prepare final step code and execute it
    QString preparedCode = code;
    preparedCode.replace("$TABLE", aggregateStorage["table"].toString()).replace("$ARGC", QString::number(argCount));

    QStringList queries = splitQueries(preparedCode, db->getDialect());
    SqlResultsPtr results;
    foreach (const QString& query, queries)
    {
        results = db->exec(query, Db::Flag::NO_LOCK);
        if (results->isError())
        {
            success = false;
            return results->getErrorText();
        }
    }

    success = true;
    return results->getSingleCell();
}

QString SqlFunctionSql::getLanguageName() const
{
    return "SQL";
}

QByteArray SqlFunctionSql::getIconData() const
{
    static const char* icon = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJ"
            "bWFnZVJlYWR5ccllPAAAARpJREFUeNrE0z9LQlEYx/Fr/gXdJILegSQJEk13yrUxxDFwUhqagyAE"
            "F8E34BsIFKlozpqao8K9WVrCJSWu30d+F44SOjj4wId77nPOeTjn3HMjQRB4m8SOt2Fsv0CE8Hle"
            "YoRdnGEPF8jhB4+4xw0ONLc7R4E7VJBX23JNtJBFSflD9YXjrT3fQhxpfKqqRQEf+MaLxuTVF3e3"
            "bo02yjjBrTMojD+9R52+sO3F8KxC13YmeELCHbT0vtBnE6ua1EfDGZRQO7qiQNEK1LiNRzynSKpj"
            "CJ+8fY1T5d/VZ+2Y5nQs8YVXPW01+zjGg3JvuFK+q1yoZ3tOaVnJpcOzqOtwzzF28rbaX0zWXbQM"
            "Bvqc/r83cet/40yAAQCHjz1eQkhXqAAAAABJRU5ErkJggg==";

    return QByteArray(icon);
}

QList<QVariant> SqlFunctionSql::getArgsForQuery(Db* db, const QString& query, const QList<QVariant>& args)
{
    TokenList tokens = Lexer::tokenize(query, db->getDialect());
    return args.mid(0, tokens.filter(Token::BIND_PARAM).size());
}
