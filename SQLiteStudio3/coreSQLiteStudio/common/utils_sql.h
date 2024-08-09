#ifndef UTILS_SQL_H
#define UTILS_SQL_H

#include "parser/token.h"
#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QChar>
#include <QPair>

// TODO: unit tests for most of methods from this module

enum class NameWrapper
{
    DOUBLE_QUOTE,
    QUOTE,
    BACK_QUOTE,
    BRACKET,
    null
};

enum class QueryAccessMode
{
    READ,
    WRITE
};

enum class SqliteDataType
{
    UNKNOWN = -1,
    _NULL = 0,
    INTEGER = 1,
    REAL = 2,
    TEXT = 3,
    BLOB = 4
};

typedef QPair<QString,QStringList> QueryWithParamNames;
typedef QPair<QString,int> QueryWithParamCount;

API_EXPORT void initUtilsSql();
API_EXPORT SqliteDataType toSqliteDataType(const QString& typeStr);
API_EXPORT bool doesObjectNeedWrapping(const QString& str);
API_EXPORT bool doesObjectNeedWrapping(const QChar& c);
API_EXPORT bool isObjectWrapped(const QChar& c);
API_EXPORT bool doesStringNeedWrapping(const QString& str);
API_EXPORT bool isStringWrapped(const QString& str);
API_EXPORT QString wrapObjIfNeeded(const QString& obj, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QString wrapObjIfNeeded(const QString& obj, bool useDoubleQuoteForEmptyValue, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QString wrapObjName(const QString& obj, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QString wrapObjName(const QString& obj, bool useDoubleQuoteForEmptyValue, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT TokenPtr stripObjName(TokenPtr token);
API_EXPORT QString stripObjName(const QString &str);
API_EXPORT QString stripObjName(QString& str);
API_EXPORT bool isObjWrapped(const QString& str);
API_EXPORT NameWrapper getObjWrapper(const QString& str);
API_EXPORT bool isWrapperChar(const QChar& c);
API_EXPORT QString wrapString(const QString& str);
API_EXPORT QStringList wrapStrings(const QStringList& strList);
API_EXPORT QString wrapStringIfNeeded(const QString& str);
API_EXPORT QString escapeString(QString &str);
API_EXPORT QString escapeString(const QString& str);
API_EXPORT QString stripString(QString& str);
API_EXPORT QString stripString(const QString& str);
API_EXPORT QString stripEndingSemicolon(const QString& str);
API_EXPORT QPair<QChar,QChar> getQuoteCharacter(QString& obj, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QList<QString> wrapObjNames(const QList<QString>& objList, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QList<QString> wrapObjNamesIfNeeded(const QList<QString>& objList, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QList<NameWrapper> getAllNameWrappers();
API_EXPORT QString wrapValueIfNeeded(const QString& str);
API_EXPORT QString wrapValueIfNeeded(const QVariant& value);
API_EXPORT TYPE_OF_QHASH qHash(NameWrapper wrapper);
API_EXPORT QString getPrefixDb(const QString& origDbName);
API_EXPORT bool isSystemTable(const QString& name);
API_EXPORT bool isSystemIndex(const QString& name);
API_EXPORT QString removeComments(const QString& value);
API_EXPORT QList<TokenList> splitQueries(const TokenList& tokenizedQueries, bool* complete = nullptr);
API_EXPORT QStringList splitQueries(const QString& sql, bool keepEmptyQueries = true, bool removeComments = false, bool* complete = nullptr);
API_EXPORT QStringList quickSplitQueries(const QString& sql, bool keepEmptyQueries = true, bool removeComments = false);
API_EXPORT QString getQueryWithPosition(const QStringList& queries, int position, int* startPos = nullptr);
API_EXPORT QString getQueryWithPosition(const QString& queries, int position, int* startPos = nullptr);
API_EXPORT QPair<int, int> getQueryBoundriesForPosition(const QString& contents, int cursorPosition, bool fallBackToPreviousIfNecessary);
API_EXPORT QList<QueryWithParamNames> getQueriesWithParamNames(const QString& query);
API_EXPORT QList<QueryWithParamCount> getQueriesWithParamCount(const QString& query);
API_EXPORT QueryWithParamNames getQueryWithParamNames(const QString& query);
API_EXPORT QueryWithParamCount getQueryWithParamCount(const QString& query);
API_EXPORT QString trimBindParamPrefix(const QString& param);
API_EXPORT QString commentAllSqlLines(const QString& sql);
API_EXPORT QString getBindTokenName(const TokenPtr& token);
API_EXPORT QueryAccessMode getQueryAccessMode(const QString& query, bool* isSelect = nullptr);
API_EXPORT QStringList valueListToSqlList(const QList<QVariant>& values);
API_EXPORT QString trimQueryEnd(const QString& query);
API_EXPORT QByteArray blobFromLiteral(const QString& value);


#endif // UTILS_SQL_H
