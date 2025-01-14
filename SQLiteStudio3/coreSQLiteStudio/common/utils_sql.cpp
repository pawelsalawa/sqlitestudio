#include "common/utils_sql.h"
#include "common/utils.h"
#include "db/sqlquery.h"
#include "parser/token.h"
#include "parser/lexer.h"
#include "parser/keywords.h"
#include <QHash>
#include <QPair>
#include <QString>
#include <QDebug>
#include <QMetaType>

QString invalidIdCharacters = "[](){}\"'@*.,+-=/#$%&|:; \t\n<>";
QHash<NameWrapper,QPair<QChar,QChar>> wrapperChars;
QHash<NameWrapper,QPair<QChar,bool>> wrapperEscapedEnding;
QList<NameWrapper> sqlite3Wrappers;
QSet<QString> sqlite3ReservedLiterals = {"true", "false"}; // true/false as column names - #5065

void initUtilsSql()
{
    wrapperChars[NameWrapper::BRACKET] = QPair<QChar,QChar>('[', ']');
    wrapperChars[NameWrapper::QUOTE] = QPair<QChar,QChar>('\'', '\'');
    wrapperChars[NameWrapper::BACK_QUOTE] = QPair<QChar,QChar>('`', '`');
    wrapperChars[NameWrapper::DOUBLE_QUOTE] = QPair<QChar,QChar>('"', '"');

    wrapperEscapedEnding[NameWrapper::BRACKET] = QPair<QChar,bool>(']', false);
    wrapperEscapedEnding[NameWrapper::QUOTE] = QPair<QChar,bool>('\'', true);
    wrapperEscapedEnding[NameWrapper::BACK_QUOTE] = QPair<QChar,bool>('`', true);
    wrapperEscapedEnding[NameWrapper::DOUBLE_QUOTE] = QPair<QChar,bool>('"', true);

    sqlite3Wrappers << NameWrapper::DOUBLE_QUOTE
                    << NameWrapper::BRACKET
                    << NameWrapper::QUOTE
                    << NameWrapper::BACK_QUOTE;

    qRegisterMetaType<SqlQueryPtr>("SqlQueryPtr");
}

bool doesObjectNeedWrapping(const QString& str)
{
    if (str.isEmpty())
        return true;

    // It used to return false if object name looked to be wrapped already,
    // but actually name [abc] is proper name that needs to be wrapped (i.e. "[abc]").
    // Bug reported for this was #4362
    //if (isObjWrapped(str))
    //    return false;

    // The "soft keyword" check added, as they don't require wrapping.
    // For example: SELECT replace('abc', 'a', '1');
    if (isKeyword(str) && !isSoftKeyword(str))
        return true;

    for (int i = 0; i < str.size(); i++)
        if (doesObjectNeedWrapping(str[i]))
            return true;

    if (str[0].isDigit())
        return true;

    if (isReservedLiteral(str))
        return true;

    return false;
}

bool isReservedLiteral(const QString& str)
{
    return sqlite3ReservedLiterals.contains(str.toLower());
}

bool doesObjectNeedWrapping(const QChar& c)
{
    return invalidIdCharacters.indexOf(c) >= 0;
}

bool isObjectWrapped(const QChar& c)
{
    return !doesObjectNeedWrapping(c);
}

QString wrapObjIfNeeded(const QString& obj, NameWrapper favWrapper)
{
    if (doesObjectNeedWrapping(obj))
        return wrapObjName(obj, favWrapper);

    return obj;
}

QString wrapObjIfNeeded(const QString& obj, bool useDoubleQuoteForEmptyValue, NameWrapper favWrapper)
{
    return wrapObjIfNeeded(obj, ((useDoubleQuoteForEmptyValue && obj.isEmpty()) ? NameWrapper::DOUBLE_QUOTE : favWrapper));
}

QString wrapObjName(const QString& obj, bool useDoubleQuoteForEmptyValue, NameWrapper favWrapper)
{
    return wrapObjName(obj, ((useDoubleQuoteForEmptyValue && obj.isEmpty()) ? NameWrapper::DOUBLE_QUOTE : favWrapper));
}

QString wrapObjName(const QString& obj, NameWrapper favWrapper)
{
    QString result =  obj;
    if (result.isNull())
        result = "";

    QPair<QChar,QChar> wrapChars = getQuoteCharacter(result, favWrapper);

    if (wrapChars.first.isNull() || wrapChars.second.isNull())
    {
        qDebug() << "No quote character possible for object name: " << result;
        return result;
    }
    result.prepend(wrapChars.first);
    result.append(wrapChars.second);
    return result;
}

QPair<QChar,QChar> getQuoteCharacter(QString& obj, NameWrapper favWrapper)
{
    QList<NameWrapper> wrappers = sqlite3Wrappers;

    // Move favourite wrapper to front of list
    if (wrappers.contains(favWrapper))
    {
        wrappers.removeOne(favWrapper);
        wrappers.insert(0, favWrapper);
    }

    QPair<QChar,QChar> wrapChars;
    for (NameWrapper wrapper : wrappers)
    {
        wrapChars = wrapperChars[wrapper];
        if (obj.indexOf(wrapChars.first) > -1)
            continue;

        if (obj.indexOf(wrapChars.second) > -1)
            continue;

        return wrapChars;
    }

    return QPair<QChar,QChar>();
}

QList<QString> wrapObjNames(const QList<QString>& objList, NameWrapper favWrapper)
{
    QList<QString> results;
    for (int i = 0; i < objList.size(); i++)
        results << wrapObjName(objList[i], favWrapper);

    return results;
}

QList<QString> wrapObjNamesIfNeeded(const QList<QString>& objList, NameWrapper favWrapper)
{
    QList<QString> results;
    for (int i = 0; i < objList.size(); i++)
        results << wrapObjIfNeeded(objList[i], favWrapper);

    return results;
}

QList<NameWrapper> getAllNameWrappers()
{
    return {NameWrapper::DOUBLE_QUOTE, NameWrapper::BRACKET, NameWrapper::BACK_QUOTE, NameWrapper::QUOTE};
}

QString wrapValueIfNeeded(const QString& str)
{
    return wrapValueIfNeeded(QVariant::fromValue(str));
}

QString wrapValueIfNeeded(const QVariant& value)
{
    if (isNumeric(value))
        return value.toString();

    return wrapString(value.toString());
}

QString wrapString(const QString& str)
{
    QString result = str;
    result.prepend("'");
    result.append("'");
    return result;
}

bool doesStringNeedWrapping(const QString& str)
{
    if (str.size() == 0)
        return false;

    return str[0] == '\'' && str[str.length()-1] == '\'';
}

bool isStringWrapped(const QString& str)
{
    return !doesStringNeedWrapping(str);
}

QString wrapStringIfNeeded(const QString& str)
{
    if (isStringWrapped(str))
        return wrapString(str);

    return str;
}

QString escapeString(QString& str)
{
    return str.replace('\'', "''");
}

QString escapeString(const QString& str)
{
    QString newStr = str;
    return newStr.replace('\'', "''");
}

QString stripString(QString& str)
{
    if (str.length() <= 1)
        return str;

    if (str[0] == '\'' && str[str.length()-1] == '\'')
        return str.mid(1, str.length()-2);

    return str;
}

QString stripString(const QString& str)
{
    QString newStr = str;
    return stripString(newStr);
}

QString stripEndingSemicolon(const QString& str)
{
    QString newStr = rStrip(str);
    if (newStr.size() == 0)
        return str;

    if (newStr[newStr.size()-1] == ';')
    {
        newStr.chop(1);
        return newStr;
    }
    else
        return str;
}

QString stripObjName(const QString &str)
{
    QString newStr = str;
    return stripObjName(newStr);
}

QString stripObjName(QString &str)
{
    if (str.isNull())
        return str;

    if (str.length() <= 1)
        return str;

    if (!isObjWrapped(str))
        return str;

    return str.mid(1, str.length()-2);
}

bool isObjWrapped(const QString& str)
{
    return getObjWrapper(str) != NameWrapper::null;
}

bool doesNotContainEndingWrapperChar(const QString& str, NameWrapper wrapper)
{
    QString innerPart = str.mid(1, str.length() - 2);
    const QChar& endingChar = wrapperEscapedEnding[wrapper].first;
    bool escapingAllowed = wrapperEscapedEnding[wrapper].second;
    int idx = -1;
    int lastIdx = innerPart.length() - 1;
    while ((idx = innerPart.indexOf(endingChar, idx + 1)) > -1)
    {
        if (idx == lastIdx || !escapingAllowed || innerPart[idx + 1] != endingChar)
            return false;

        idx++; // we had occurrence, but it was escaped, so we need to skip the second (escape) char
    }
    return true;
}

NameWrapper getObjWrapper(const QString& str)
{
    if (str.isEmpty())
        return NameWrapper::null;

    for (NameWrapper wrapper : sqlite3Wrappers)
    {
        QPair<QChar,QChar> chars = wrapperChars[wrapper];
        if (str[0] == chars.first && str[str.length()-1] == chars.second && doesNotContainEndingWrapperChar(str, wrapper))
            return wrapper;
    }
    return NameWrapper::null;
}

bool isWrapperChar(const QChar& c)
{
    for (NameWrapper wrapper : sqlite3Wrappers)
    {
        QPair<QChar,QChar> chars = wrapperChars[wrapper];
        if (c == chars.first || c == chars.second)
            return true;
    }
    return false;
}

size_t qHash(NameWrapper wrapper)
{
    return (size_t)wrapper;
}

QString getPrefixDb(const QString& origDbName)
{
    if (origDbName.isEmpty())
        return "main";
    else
        return wrapObjIfNeeded(origDbName);
}

bool isSystemTable(const QString &name)
{
    return name.startsWith("sqlite_");
}

bool isSystemIndex(const QString &name)
{
    return name.startsWith("sqlite_autoindex_");
}


TokenPtr stripObjName(TokenPtr token)
{
    if (!token)
        return token;

    token->value = stripObjName(token->value);
    return token;
}

QString removeComments(const QString& value)
{
    Lexer lexer;
    TokenList tokens = lexer.tokenize(value);
    while (tokens.remove(Token::COMMENT))
        continue;

    return tokens.detokenize();
}

void splitQueriesUpdateCaseWhenDepth(Token::Type type, const QString& value, int& caseWhenDepth)
{
    if (type != Token::KEYWORD)
        return;

    if (value == "CASE")
        caseWhenDepth++;
    else if (value == "END" && caseWhenDepth > 0)
        caseWhenDepth--;
}

QList<TokenList> splitQueries(const TokenList& tokenizedQuery, bool* complete)
{
    QList<TokenList> queries;
    TokenList currentQueryTokens;
    QString value;
    int caseWhenDepth = 0;
    int createTriggerMeter = 0;
    bool insideTrigger = false;
    bool completeQuery = false;
    for (const TokenPtr& token : tokenizedQuery)
    {
        value = token->value.toUpper();
        if (!token->isWhitespace())
            completeQuery = false;

        if (insideTrigger)
        {
            if (token->type == Token::KEYWORD && value == "END" && caseWhenDepth <= 0 && caseWhenDepth == 0)
            {
                insideTrigger = false;
                completeQuery = true;
            }

            currentQueryTokens << token;
            splitQueriesUpdateCaseWhenDepth(token->type, value, caseWhenDepth);
            continue;
        }

        splitQueriesUpdateCaseWhenDepth(token->type, value, caseWhenDepth);

        if (token->type == Token::KEYWORD)
        {
            if (value == "CREATE" || value == "TRIGGER" || value == "BEGIN")
                createTriggerMeter++;

            if (createTriggerMeter == 3)
                insideTrigger = true;

            currentQueryTokens << token;
        }
        else if (token->type == Token::OPERATOR && value == ";")
        {
            createTriggerMeter = 0;
            caseWhenDepth = 0;
            currentQueryTokens << token;
            queries << currentQueryTokens;
            currentQueryTokens.clear();
            completeQuery = true;
        }
        else
        {
            currentQueryTokens << token;
        }
    }

    if (currentQueryTokens.size() > 0)
        queries << currentQueryTokens;

    if (complete)
        *complete = completeQuery;

    return queries;
}

QStringList quickSplitQueries(const QString& sql, bool keepEmptyQueries, bool removeComments)
{
    QChar c;
    bool inString = false;
    bool inMultiLineComment = false;
    bool inSingleLineComment = false;
    QStringList queries;
    QString query;
    QString trimmed;
    for (int i = 0, total = sql.size(); i < total; ++i)
    {
        c = sql[i];

        // String
        if (inString)
        {
            query += c;
            if (c == '\'')
            {
                inString = false;
            }
            continue;
        }

        // One-line comment
        if (inSingleLineComment)
        {
            if (!removeComments)
                query += c;

            if (c == '\r' && (i + 1) < total && sql[i+1] == '\n')
            {
                if (!removeComments)
                    query += '\n';

                i++;
                inSingleLineComment = false;
            }
            else if (c == '\n' || c == '\r')
                inSingleLineComment = false;

            continue;
        }

        // Multi-line comment
        if (inMultiLineComment)
        {
            if (!removeComments)
                query += c;

            if (c == '*' && (i + 1) < total && sql[i+1] == '/')
            {
                if (!removeComments)
                    query += '/';

                i++;
                inMultiLineComment = false;
            }

            continue;
        }

        // Everything rest
        if (c == '\'')
        {
            query += c;
            inString = true;
        }
        else if (c == '-' && (i + 1) < total && sql[i+1] == '-')
        {
            inSingleLineComment = true;
            i++;
            if (!removeComments)
                query += "--";
        }
        else if (c == '/' && (i + 1) < total && sql[i+1] == '*')
        {
            inMultiLineComment = true;
            i++;
            if (!removeComments)
                query += "/*";
        }
        else if (c == ';')
        {
            query += c;
            if (keepEmptyQueries || (!(trimmed = query.trimmed()).isEmpty() && trimmed != ";"))
                queries << query;

            query.clear();
        }
        else
        {
            query += c;
        }
    }

    if (!query.isNull() && (!(trimmed = query.trimmed()).isEmpty() && trimmed != ";"))
        queries << query;

    return queries;
}

QStringList splitQueries(const QString& sql, bool keepEmptyQueries, bool removeComments, bool* complete)
{
    TokenList tokens = Lexer::tokenize(sql);
    if (removeComments)
        tokens = tokens.filterOut(Token::COMMENT);

    QList<TokenList> tokenizedQueries = splitQueries(tokens, complete);

    QString query;
    QStringList queries;
    for (const TokenList& queryTokens : tokenizedQueries)
    {
        query = queryTokens.detokenize();
        if (keepEmptyQueries || (!query.trimmed().isEmpty() && query.trimmed() != ";"))
            queries << query;
    }

    return queries;
}

QString getQueryWithPosition(const QStringList& queries, int position, int* startPos)
{
    int currentPos = 0;
    int length = 0;

    if (startPos)
        *startPos = 0;

    for (const QString& query : queries)
    {
        length = query.length();
        if (position >= currentPos && position < currentPos+length)
            return query;

        currentPos += length;

        if (startPos)
            *startPos += length;
    }

    // If we passed all queries and it happens that the cursor is just after last query - this is the query we want.
    if (position == currentPos && queries.size() > 0)
    {
        if (startPos)
            *startPos -= length;

        return queries.last();
    }

    if (startPos)
        *startPos = -1;

    return QString();
}

int getCursorFinalPositionForQueries(const QString& queries, int position)
{
    const static QSet<QChar> whitespaceChars = {' ', '\t'};

    int len = queries.length();
    if (position >= len || position < 1)
        return position;

    if (!whitespaceChars.contains(queries[position]) && queries[position] != '\xa')
        return position;

    // First checking characters at & after cursor - are they just whitespaces until end of line?
    int newPos = position;
    while (newPos < len)
    {
        QChar c = queries[newPos++];
        if (whitespaceChars.contains(c))
            continue;
        else if (c == '\xa')
            break;
        else
            return position;
    }

    // Okay, only whitespaces after cursor, so let's check whats before cursor,
    // until the ';' - is it just whitespaces too?
    newPos = position;
    while (newPos > 1 && whitespaceChars.contains(queries[newPos - 1]))
        newPos--;

    if (queries[newPos - 1] == ';')
    {
        // Jackpot! Our cursor is just somewhere in whitespaces after the query.
        // Let's consider that prior query as current.
        return newPos - 1;
    }

    return position;
}

QString getQueryWithPosition(const QString& queries, int position, int* startPos)
{
    position = getCursorFinalPositionForQueries(queries, position);
    QStringList queryList = splitQueries(queries);
    return getQueryWithPosition(queryList, position, startPos);
}

QPair<int, int> getQueryBoundriesForPosition(const QString& contents, int cursorPosition, bool fallBackToPreviousIfNecessary)
{
    int queryStartPos;
    QString query = getQueryWithPosition(contents, cursorPosition, &queryStartPos);
    TokenList tokens = Lexer::tokenize(query);
    tokens.trim();
    tokens.trimRight(Token::OPERATOR, ";");

    if (tokens.size() == 0 && fallBackToPreviousIfNecessary)
    {
        // Fallback
        cursorPosition = contents.lastIndexOf(";", cursorPosition - 1);
        if (cursorPosition > -1)
        {
            query = getQueryWithPosition(contents, cursorPosition, &queryStartPos);
            tokens = Lexer::tokenize(query);
            tokens.trim();
            tokens.trimRight(Token::OPERATOR, ";");
        }
    }

    if (tokens.size() == 0)
        return QPair<int, int>(-1, -1);

    return QPair<int, int>(tokens.first()->start + queryStartPos, tokens.last()->end + 1 + queryStartPos);
}

QString trimBindParamPrefix(const QString& param)
{
    if (param == "?")
        return param;

    if (param.startsWith("$") || param.startsWith("@") || param.startsWith(":") || param.startsWith("?"))
        return param.mid(1);

    return param;
}

QList<QueryWithParamNames> getQueriesWithParamNames(const QString& query)
{
    QList<QueryWithParamNames> results;

    TokenList allTokens = Lexer::tokenize(query);
    QList<TokenList> queries = splitQueries(allTokens);

    QString queryStr;
    QStringList paramNames;
    for (const TokenList& tokens : queries)
    {
        paramNames.clear();
        for (const TokenPtr& token : tokens.filter(Token::BIND_PARAM))
            paramNames << token->value;

        queryStr = tokens.detokenize().trimmed();
        if (!queryStr.isEmpty())
            results << QueryWithParamNames(queryStr, paramNames);
    }
    return results;
}

QList<QueryWithParamCount> getQueriesWithParamCount(const QString& query)
{
    QList<QueryWithParamCount> results;

    TokenList allTokens = Lexer::tokenize(query);
    QList<TokenList> queries = splitQueries(allTokens);

    QString queryStr;
    for (const TokenList& tokens : queries)
    {
        queryStr = tokens.detokenize().trimmed();
        if (!queryStr.isEmpty())
            results << QueryWithParamCount(queryStr, tokens.filter(Token::BIND_PARAM).size());
    }

    return results;
}

QueryWithParamNames getQueryWithParamNames(const QString& query)
{
    TokenList allTokens = Lexer::tokenize(query);

    QStringList paramNames;
    for (const TokenPtr& token : allTokens.filter(Token::BIND_PARAM))
        paramNames << token->value;

    return QueryWithParamNames(query, paramNames);
}

QueryWithParamCount getQueryWithParamCount(const QString& query)
{
    TokenList allTokens = Lexer::tokenize(query);
    return QueryWithParamCount(query, allTokens.filter(Token::BIND_PARAM).size());
}

QString commentAllSqlLines(const QString& sql)
{
    QStringList lines = splitByLines(sql);
    QMutableStringListIterator it(lines);
    while (it.hasNext())
        it.next().prepend("-- ");

    return joinLines(lines);
}

QString getBindTokenName(const TokenPtr& token)
{
    if (token->type != Token::BIND_PARAM)
        return QString();

    if (token->value == "?")
        return token->value;

    return token->value.mid(1);
}

QueryAccessMode getQueryAccessMode(const QString& query, bool* isSelect)
{
    static QStringList readOnlyCommands = {"ANALYZE", "EXPLAIN", "PRAGMA", "SELECT"};

    if (isSelect)
        *isSelect = false;

    TokenList tokens = Lexer::tokenize(query);
    int keywordIdx = tokens.indexOf(Token::KEYWORD);
    if (keywordIdx < 0)
        return QueryAccessMode::WRITE;

    int cmdIdx = readOnlyCommands.indexOf(tokens[keywordIdx]->value.toUpper());
    if (keywordIdx > -1 && cmdIdx > -1)
    {
        if (cmdIdx == 3 && isSelect)
            *isSelect = true;

        return QueryAccessMode::READ;
    }

    if (keywordIdx > -1 && tokens[keywordIdx]->value.toUpper() == "WITH")
    {
        bool matched = false;
        bool queryIsSelect = false;
        int depth = 0;
        for (TokenPtr token : tokens)
        {
            switch (token->type)
            {
                case Token::PAR_LEFT:
                    depth++;
                    break;
                case Token::PAR_RIGHT:
                    depth--;
                    break;
                case Token::KEYWORD:
                    if (depth == 0)
                    {
                        QString val = token->value.toUpper();
                        if (val == "SELECT")
                        {
                            matched = true;
                            queryIsSelect = true;
                        }
                        else if (val == "DELETE" || val == "UPDATE" || val == "INSERT")
                        {
                            matched = true;
                        }
                    }
                    break;
                default:
                    break;
            }

            if (matched)
                break;
        }

        if (queryIsSelect)
        {
            if (isSelect)
                *isSelect = true;

            return QueryAccessMode::READ;
        }
    }

    return QueryAccessMode::WRITE;
}

QStringList valueListToSqlList(const QVariantList& values)
{
    QStringList argList;
    for (const QVariant& value : values)
        argList << valueToSqlLiteral(value);

    return argList;
}

QString valueToSqlLiteral(const QVariant& value)
{
    if (!value.isValid() || value.isNull())
        return "NULL";

    switch (value.userType())
    {
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            return value.toString();
            break;
        case QMetaType::Double:
            return doubleToString(value);
            break;
        case QMetaType::Bool:
            return QString::number(value.toInt());
            break;
        case QMetaType::QByteArray:
            return "X'" + value.toByteArray().toHex().toUpper() + "'";
            break;
        default:
            break;
    }
    return wrapString(escapeString(value.toString()));
}

QStringList wrapStrings(const QStringList& strList)
{
    QStringList list;
    for (const QString& str : strList)
        list << wrapString(str);

    return list;
}

QString trimQueryEnd(const QString &query)
{
    QString q = query.trimmed();
    while (q.endsWith(";"))
    {
        q.chop(1);
        q = q.trimmed();
    }
    return q;
}

SqliteDataType toSqliteDataType(const QString& typeStr)
{
    QString upperType = typeStr.trimmed().toUpper();
    if (upperType == "INTEGER")
        return SqliteDataType::INTEGER;

    if (upperType == "REAL")
        return SqliteDataType::REAL;

    if (upperType == "TEXT")
        return SqliteDataType::TEXT;

    if (upperType == "BLOB")
        return SqliteDataType::BLOB;

    if (upperType == "NULL")
        return SqliteDataType::_NULL;

    return SqliteDataType::UNKNOWN;
}

QByteArray blobFromLiteral(const QString& value)
{
    if (value.length() <= 3)
    {
        qCritical() << "Call to blobFromLiteral() with blob literal shorter or equal to 3 characters:" << value;
        return QByteArray();
    }

    QString hex = value.mid(2, value.length() - 3);
    return QByteArray::fromHex(hex.toLatin1());
}
