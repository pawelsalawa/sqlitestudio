#include "common/utils_sql.h"
#include "common/utils.h"
#include "db/sqlquery.h"
#include "parser/token.h"
#include "parser/lexer.h"
#include "parser/keywords.h"
#include "log.h"
#include <QHash>
#include <QPair>
#include <QString>
#include <QDebug>
#include <QMetaType>

QString invalidIdCharacters = "[]()\"'@*.,+-=/%&|:; \t\n<>";
QHash<NameWrapper,QPair<QChar,QChar> > wrapperChars;
QList<NameWrapper> sqlite3Wrappers;
QList<NameWrapper> sqlite2Wrappers;

void initUtilsSql()
{
    wrapperChars[NameWrapper::BRACKET] = QPair<QChar,QChar>('[', ']');
    wrapperChars[NameWrapper::QUOTE] = QPair<QChar,QChar>('\'', '\'');
    wrapperChars[NameWrapper::BACK_QUOTE] = QPair<QChar,QChar>('`', '`');
    wrapperChars[NameWrapper::DOUBLE_QUOTE] = QPair<QChar,QChar>('"', '"');

    sqlite3Wrappers << NameWrapper::DOUBLE_QUOTE
                    << NameWrapper::BRACKET
                    << NameWrapper::QUOTE
                    << NameWrapper::BACK_QUOTE;
    sqlite2Wrappers << NameWrapper::DOUBLE_QUOTE
                    << NameWrapper::BRACKET
                    << NameWrapper::QUOTE;

    qRegisterMetaType<SqlQueryPtr>("SqlQueryPtr");
}

bool doesObjectNeedWrapping(const QString& str, Dialect dialect)
{
    if (str.isEmpty())
        return true;

    if (isObjWrapped(str, dialect))
        return false;

    if (isKeyword(str, dialect))
        return true;

    for (int i = 0; i < str.size(); i++)
        if (doesObjectNeedWrapping(str[i]))
            return true;

    if (str[0].isDigit())
        return true;

    return false;
}

bool doesObjectNeedWrapping(const QChar& c)
{
    return invalidIdCharacters.indexOf(c) >= 0;
}

bool isObjectWrapped(const QChar& c, Dialect dialect)
{
    return !doesObjectNeedWrapping(c, dialect);
}

bool isObjectWrapped(const QChar& c)
{
    return !doesObjectNeedWrapping(c);
}

QString wrapObjIfNeeded(const QString& obj, Dialect dialect, NameWrapper favWrapper)
{
    if (doesObjectNeedWrapping(obj, dialect))
        return wrapObjName(obj, dialect, favWrapper);

    return obj;
}

QString wrapObjIfNeeded(const QString& obj, Dialect dialect, bool useDoubleQuoteForEmptyValue, NameWrapper favWrapper)
{
    return wrapObjIfNeeded(obj, dialect, ((useDoubleQuoteForEmptyValue && obj.isEmpty()) ? NameWrapper::DOUBLE_QUOTE : favWrapper));
}

QString wrapObjName(const QString& obj, Dialect dialect, bool useDoubleQuoteForEmptyValue, NameWrapper favWrapper)
{
    return wrapObjName(obj, dialect, ((useDoubleQuoteForEmptyValue && obj.isEmpty()) ? NameWrapper::DOUBLE_QUOTE : favWrapper));
}

QString wrapObjName(const QString& obj, Dialect dialect, NameWrapper favWrapper)
{
    QString result =  obj;
    if (result.isNull())
        result = "";

    QPair<QChar,QChar> wrapChars = getQuoteCharacter(result, dialect, favWrapper);

    if (wrapChars.first.isNull() || wrapChars.second.isNull())
    {
        qDebug() << "No quote character possible for object name: " << result;
        return result;
    }
    result.prepend(wrapChars.first);
    result.append(wrapChars.second);
    return result;
}

QString wrapObjName(const QString& obj, NameWrapper wrapper)
{
    QString result =  obj;
    if (wrapper == NameWrapper::null)
        return result;

    result.prepend(wrapperChars[wrapper].first);
    result.append(wrapperChars[wrapper].second);
    return result;
}

QPair<QChar,QChar> getQuoteCharacter(QString& obj, Dialect dialect, NameWrapper favWrapper)
{
    QList<NameWrapper> wrappers = (dialect == Dialect::Sqlite3) ? sqlite3Wrappers : sqlite2Wrappers;

    // Move favourite wrapper to front of list
    if (wrappers.contains(favWrapper))
    {
        wrappers.removeOne(favWrapper);
        wrappers.insert(0, favWrapper);
    }

    QPair<QChar,QChar> wrapChars;
    foreach (NameWrapper wrapper, wrappers)
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

QList<QString> wrapObjNames(const QList<QString>& objList, Dialect dialect, NameWrapper favWrapper)
{
    QList<QString> results;
    for (int i = 0; i < objList.size(); i++)
        results << wrapObjName(objList[i], dialect, favWrapper);

    return results;
}

QList<QString> wrapObjNamesIfNeeded(const QList<QString>& objList, Dialect dialect, NameWrapper favWrapper)
{
    QList<QString> results;
    for (int i = 0; i < objList.size(); i++)
        results << wrapObjIfNeeded(objList[i], dialect, favWrapper);

    return results;
}

QList<NameWrapper> getAllNameWrappers(Dialect dialect)
{
    if (dialect == Dialect::Sqlite3)
        return {NameWrapper::DOUBLE_QUOTE, NameWrapper::BRACKET, NameWrapper::BACK_QUOTE, NameWrapper::QUOTE};
    else
        return {NameWrapper::DOUBLE_QUOTE, NameWrapper::BRACKET, NameWrapper::QUOTE};
}

QString wrapValueIfNeeded(const QString& str)
{
    return wrapValueIfNeeded(QVariant::fromValue(str));
}

QString wrapValueIfNeeded(const QVariant& value)
{
    if (value.canConvert(QMetaType::LongLong) || value.canConvert(QMetaType::Double))
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

QString stripObjName(const QString &str, Dialect dialect)
{
    QString newStr = str;
    return stripObjName(newStr, dialect);
}

QString stripObjName(QString &str, Dialect dialect)
{
    if (str.isNull())
        return str;

    if (str.length() <= 1)
        return str;

    if (!isObjWrapped(str, dialect))
        return str;

    return str.mid(1, str.length()-2);
}

bool isObjWrapped(const QString& str, Dialect dialect)
{
    return getObjWrapper(str, dialect) != NameWrapper::null;
}

NameWrapper getObjWrapper(const QString& str, Dialect dialect)
{
    if (str.isEmpty())
        return NameWrapper::null;

    QList<NameWrapper> wrappers;

    if (dialect == Dialect::Sqlite2)
        wrappers = sqlite2Wrappers;
    else
        wrappers = sqlite3Wrappers;

    foreach (NameWrapper wrapper, wrappers)
    {
        QPair<QChar,QChar> chars = wrapperChars[wrapper];
        if (str[0] == chars.first && str[str.length()-1] == chars.second)
            return wrapper;
    }
    return NameWrapper::null;
}

bool isWrapperChar(const QChar& c, Dialect dialect)
{
    QList<NameWrapper> wrappers;
    if (dialect == Dialect::Sqlite2)
        wrappers = sqlite2Wrappers;
    else
        wrappers = sqlite3Wrappers;

    foreach (NameWrapper wrapper, wrappers)
    {
        QPair<QChar,QChar> chars = wrapperChars[wrapper];
        if (c == chars.first || c == chars.second)
            return true;
    }
    return false;
}

int qHash(NameWrapper wrapper)
{
    return (uint)wrapper;
}

QString getPrefixDb(const QString& origDbName, Dialect dialect)
{
    if (origDbName.isEmpty())
        return "main";
    else
        return wrapObjIfNeeded(origDbName, dialect);
}

bool isSystemTable(const QString &name)
{
    return name.startsWith("sqlite_");
}

bool isSystemIndex(const QString &name, Dialect dialect)
{
    switch (dialect)
    {
        case Dialect::Sqlite3:
            return name.startsWith("sqlite_autoindex_");
        case Dialect::Sqlite2:
        {
            QRegExp re("*(*autoindex*)*");
            re.setPatternSyntax(QRegExp::Wildcard);
            return re.exactMatch(name);
        }
    }
    return false;
}


TokenPtr stripObjName(TokenPtr token, Dialect dialect)
{
    if (!token)
        return token;

    token->value = stripObjName(token->value, dialect);
    return token;
}

QString removeComments(const QString& value)
{
    Lexer lexer(Dialect::Sqlite3);
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
    foreach (const TokenPtr& token, tokenizedQuery)
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

QStringList splitQueries(const QString& sql, Dialect dialect, bool keepEmptyQueries, bool removeComments, bool* complete)
{
    TokenList tokens = Lexer::tokenize(sql, dialect);
    if (removeComments)
        tokens = tokens.filterOut(Token::COMMENT);

    QList<TokenList> tokenizedQueries = splitQueries(tokens, complete);

    QString query;
    QStringList queries;
    foreach (const TokenList& queryTokens, tokenizedQueries)
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

    return QString::null;
}

QString getQueryWithPosition(const QString& queries, int position, int* startPos)
{
    QStringList queryList = quickSplitQueries(queries);
    return getQueryWithPosition(queryList, position, startPos);
}

QString trimBindParamPrefix(const QString& param)
{
    if (param == "?")
        return param;

    if (param.startsWith("$") || param.startsWith("@") || param.startsWith(":") || param.startsWith("?"))
        return param.mid(1);

    return param;
}

QList<QueryWithParamNames> getQueriesWithParamNames(const QString& query, Dialect dialect)
{
    QList<QueryWithParamNames> results;

    TokenList allTokens = Lexer::tokenize(query, dialect);
    QList<TokenList> queries = splitQueries(allTokens);

    QString queryStr;
    QStringList paramNames;
    foreach (const TokenList& tokens, queries)
    {
        paramNames.clear();
        foreach (const TokenPtr& token, tokens.filter(Token::BIND_PARAM))
            paramNames << token->value;

        queryStr = tokens.detokenize().trimmed();
        if (!queryStr.isEmpty())
            results << QueryWithParamNames(queryStr, paramNames);
    }
    return results;
}

QList<QueryWithParamCount> getQueriesWithParamCount(const QString& query, Dialect dialect)
{
    QList<QueryWithParamCount> results;

    TokenList allTokens = Lexer::tokenize(query, dialect);
    QList<TokenList> queries = splitQueries(allTokens);

    QString queryStr;
    foreach (const TokenList& tokens, queries)
    {
        queryStr = tokens.detokenize().trimmed();
        if (!queryStr.isEmpty())
            results << QueryWithParamCount(queryStr, tokens.filter(Token::BIND_PARAM).size());
    }

    return results;
}

QueryWithParamNames getQueryWithParamNames(const QString& query, Dialect dialect)
{
    TokenList allTokens = Lexer::tokenize(query, dialect);

    QStringList paramNames;
    foreach (const TokenPtr& token, allTokens.filter(Token::BIND_PARAM))
        paramNames << token->value;

    return QueryWithParamNames(query, paramNames);
}

QueryWithParamCount getQueryWithParamCount(const QString& query, Dialect dialect)
{
    TokenList allTokens = Lexer::tokenize(query, dialect);
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

QueryAccessMode getQueryAccessMode(const QString& query, Dialect dialect, bool* isSelect)
{
    static QStringList readOnlyCommands = {"ANALYZE", "EXPLAIN", "PRAGMA", "SELECT"};

    if (isSelect)
        *isSelect = false;

    TokenList tokens = Lexer::tokenize(query, dialect);
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

QStringList valueListToSqlList(const QVariantList& values, Dialect dialect)
{
    QStringList argList;
    for (const QVariant& value : values)
    {
        if (!value.isValid() || value.isNull())
        {
            argList << "NULL";
            continue;
        }

        switch (value.userType())
        {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                argList << value.toString();
                break;
            case QVariant::Double:
                argList << doubleToString(value.toDouble());
                break;
            case QVariant::Bool:
                argList << QString::number(value.toInt());
                break;
            case QVariant::ByteArray:
            {
                if (dialect == Dialect::Sqlite3) // version 2 will go to the regular string processing
                {
                    argList << "X'" + value.toByteArray().toHex().toUpper() + "'";
                    break;
                }
            }
            default:
                argList << wrapString(escapeString(value.toString()));
                break;
        }
    }
    return argList;
}

QStringList wrapStrings(const QStringList& strList)
{
    QStringList list;
    for (const QString& str : strList)
        list << wrapString(str);

    return list;
}
