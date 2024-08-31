#include "dbandroidconnection.h"
#include "sqlqueryandroid.h"
#include "sqlresultrowandroid.h"
#include "parser/lexer.h"
#include "db/sqlerrorcodes.h"
#include "common/utils_sql.h"
#include "log.h"
#include "dbandroidinstance.h"
#include <QDebug>

SqlQueryAndroid::SqlQueryAndroid(DbAndroidInstance* db, DbAndroidConnection* connection, const QString& query) :
    db(db), connection(connection), queryString(query)
{
    tokenizedQuery = Lexer::tokenize(query);
}

SqlQueryAndroid::~SqlQueryAndroid()
{
}

QString SqlQueryAndroid::getErrorText()
{
    return errorText;
}

int SqlQueryAndroid::getErrorCode()
{
    return errorCode;
}

QStringList SqlQueryAndroid::getColumnNames()
{
    return resultColumns;
}

int SqlQueryAndroid::columnCount()
{
    return resultColumns.size();
}

void SqlQueryAndroid::rewind()
{
    currentRow = -1;
}

SqlResultsRowPtr SqlQueryAndroid::nextInternal()
{
    if (resultDataList.size() == 0)
        return SqlResultsRowPtr();

    currentRow++;
    SqlResultRowAndroid* resultRow = new SqlResultRowAndroid(resultDataMap[currentRow], resultDataList[currentRow]);
    return SqlResultsRowPtr(resultRow);
}

bool SqlQueryAndroid::hasNextInternal()
{
    return (currentRow + 1 < resultDataList.size());
}

bool SqlQueryAndroid::execInternal(const QList<QVariant>& args)
{
    resetResponse();
    logSql(db, queryString, args, flags);

    int argIdx = 0;
    QString query;
    for (const TokenPtr& token : tokenizedQuery)
    {
        if (token->type != Token::BIND_PARAM)
        {
            query += token->value;
            continue;
        }

        query += convertArg(args[argIdx++]);
    }

    return executeAndHandleResponse(query);
}

bool SqlQueryAndroid::execInternal(const QHash<QString, QVariant>& args)
{
    resetResponse();
    logSql(db, queryString, args, flags);

    QString argName;
    QString query;
    for (const TokenPtr& token : tokenizedQuery)
    {
        if (token->type != Token::BIND_PARAM)
        {
            query += token->value;
            continue;
        }

        argName = token->value;
        if (!args.contains(argName))
        {
            errorCode = SqlErrorCode::OTHER_EXECUTION_ERROR;
            errorText = QObject::tr("Cannot bind argument '%1' of the query, because it's value is missing.").arg(argName);
            return false;
        }

        query += convertArg(args[argName]);
    }

    return executeAndHandleResponse(query);
}

QString SqlQueryAndroid::convertArg(const QVariant& value)
{
    if (value.isNull() || !value.isValid())
        return "NULL";

    switch (value.userType())
    {
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
        case QMetaType::Double:
            return value.toString();
        case QMetaType::QString:
            return "'" + value.toString().replace("'", "''")  + "'";
        case QMetaType::QByteArray:
            return "x'" + value.toByteArray().toHex() + "'";
        default:
            break;
    }

    qCritical() << "Unhandled argument type in SqlQueryAndroid::convertArg():"
                << static_cast<QMetaType::Type>(value.userType());
    return "";
}

bool SqlQueryAndroid::executeAndHandleResponse(const QString& query)
{
    DbAndroidConnection::ExecutionResult results = connection->executeQuery(query);
    if (results.wasError)
    {
        errorCode = (results.errorCode != 0) ? results.errorCode : SqlErrorCode::OTHER_EXECUTION_ERROR;
        errorText = results.errorMsg;
        return false;
    }

    resultColumns = results.resultColumns;
    resultDataMap = results.resultDataMap;
    resultDataList = results.resultDataList;
    return true;
}

void SqlQueryAndroid::resetResponse()
{
    resultColumns.clear();
    resultDataMap.clear();
    resultDataList.clear();
    currentRow = -1;
    errorCode = 0;
    errorText = QString();
}
