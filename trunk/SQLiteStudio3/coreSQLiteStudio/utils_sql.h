#ifndef UTILS_SQL_H
#define UTILS_SQL_H

#include "dialect.h"
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

API_EXPORT void initUtilsSql();
API_EXPORT bool doesObjectNeedWrapping(const QString& str, Dialect dialect);
API_EXPORT bool doesObjectNeedWrapping(const QChar& c);
API_EXPORT bool isObjectWrapped(const QChar& c, Dialect dialect);
API_EXPORT bool isObjectWrapped(const QChar& c);
API_EXPORT bool doesStringNeedWrapping(const QString& str);
API_EXPORT bool isStringWrapped(const QString& str);
API_EXPORT QString wrapObjIfNeeded(const QString& obj, Dialect dialect, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QString wrapObjName(const QString& obj, Dialect dialect, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT TokenPtr stripObjName(TokenPtr token, Dialect dialect);
API_EXPORT QString stripObjName(const QString &str, Dialect dialect);
API_EXPORT QString stripObjName(QString& str, Dialect dialect);
API_EXPORT bool isObjWrapped(const QString& str, Dialect dialect);
API_EXPORT NameWrapper getObjWrapper(const QString& str, Dialect dialect);
API_EXPORT bool isWrapperChar(const QChar& c, Dialect dialect);
API_EXPORT QString wrapString(const QString& str);
API_EXPORT QString wrapStringIfNeeded(const QString& str);
API_EXPORT QString escapeString(QString &str);
API_EXPORT QString escapeString(const QString& str);
API_EXPORT QString stripString(QString& str);
API_EXPORT QString stripString(const QString& str);
API_EXPORT QString stripEndingSemicolon(const QString& str);
API_EXPORT QPair<QChar,QChar> getQuoteCharacter(QString& obj, Dialect dialect,
                                     NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QList<QString> wrapObjNames(const QList<QString>& objList, Dialect dialect = Dialect::Sqlite3, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT QList<QString> wrapObjNamesIfNeeded(const QList<QString>& objList, Dialect dialect, NameWrapper favWrapper = NameWrapper::null);
API_EXPORT int qHash(NameWrapper wrapper);
API_EXPORT QString getPrefixDb(const QString& origDbName, Dialect dialect);
API_EXPORT bool isSystemTable(const QString& name);
API_EXPORT bool isSystemIndex(const QString& name, Dialect dialect);
API_EXPORT QString removeComments(const QString& value);
API_EXPORT QStringList splitQueries(const QString& sql, Dialect dialect, bool* complete = nullptr);
API_EXPORT QString getQueryWithPosition(const QStringList& queries, int position, int* startPos = nullptr);
API_EXPORT QString getQueryWithPosition(const QString& queries, int position, Dialect dialect, int* startPos = nullptr);

#endif // UTILS_SQL_H
