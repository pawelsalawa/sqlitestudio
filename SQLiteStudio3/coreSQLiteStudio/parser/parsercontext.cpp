#include "parsercontext.h"
#include "parsererror.h"
#include "lexer.h"
#include <QDebug>

ParserContext::~ParserContext()
{
    cleanUp();
}

void ParserContext::addQuery(SqliteQuery *query)
{
    parsedQueries << SqliteQueryPtr(query);
}

void ParserContext::error(TokenPtr token, const QString &text)
{
    errors << new ParserError(token, text);
    successful = false;
}

void ParserContext::error(Token* token, const QString& text)
{
    errors << new ParserError(getTokenPtr(token), text);
    successful = false;
}

void ParserContext::error(const QString &text)
{
    errors << new ParserError(text);
    successful = false;
}

void ParserContext::minorErrorAfterLastToken(const QString &text)
{
    if (ignoreMinorErrors)
        return;

    if (managedTokens.isEmpty())
    {
        qCritical() << "Tried to report minor error after last token, but there's no tokens!";
        return;
    }

    error(managedTokens.last(), text);
}

void ParserContext::minorErrorBeforeNextToken(const QString &text)
{
    if (ignoreMinorErrors)
        return;

    raiseErrorBeforeNextToken = true;
    nextTokenError = text;
}

void ParserContext::errorAfterLastToken(const QString& text)
{
    if (managedTokens.isEmpty())
    {
        qCritical() << "Tried to report error after last token, but there's no tokens!";
        return;
    }

    error(managedTokens.last(), text);
}

void ParserContext::errorBeforeNextToken(const QString& text)
{
    raiseErrorBeforeNextToken = true;
    nextTokenError = text;
}

void ParserContext::errorAtToken(const QString& text, int pos)
{
    if (managedTokens.isEmpty())
    {
        qCritical() << "Tried to report error at token" << pos << ", but there's no tokens!";
        return;
    }

    int idx = managedTokens.size() - 1 + pos;
    if (idx < 0 && idx >= managedTokens.size())
    {
        qCritical() << "Tried to report error at token" << pos << ", calculated idx was out of range:" << idx
                    << "(manages tokens size:" << managedTokens.size() << ").";
        return;
    }

    error(managedTokens[idx], text);
}

TokenPtr ParserContext::getTokenPtr(Token* token)
{
    if (tokenPtrMap.contains(token))
        return tokenPtrMap[token];

    TokenPtr tokenPtr = Lexer::getEveryTokenTypePtr(token);
    if (!tokenPtr.isNull())
        return tokenPtr;

    qWarning() << "No TokenPtr for Token*. Token asked:" << token->toString();
    return TokenPtr();
}

TokenList ParserContext::getTokenPtrList(const QList<Token*>& tokens)
{
    TokenList resList;
    foreach (Token* token, tokens)
        resList << getTokenPtr(token);

    return resList;
}

void ParserContext::addManagedToken(TokenPtr token)
{
    managedTokens << token;
    tokenPtrMap[token.data()] = token;

    if (raiseErrorBeforeNextToken)
    {
        error(token, nextTokenError);
        nextTokenError = QString::null;
        raiseErrorBeforeNextToken = false;
    }
}

bool ParserContext::isSuccessful() const
{
    return successful;
}

const QList<SqliteQueryPtr>& ParserContext::getQueries()
{
    return parsedQueries;
}

const QList<ParserError *> &ParserContext::getErrors()
{
    return errors;
}

void ParserContext::cleanUp()
{
    foreach (ParserError* err, errors)
        delete err;

    parsedQueries.clear();
    errors.clear();
    managedTokens.clear();
    nextTokenError.clear();
    tokenPtrMap.clear();
    raiseErrorBeforeNextToken = false;
    successful = true;
}

bool ParserContext::isManagedToken(Token* token)
{
    return tokenPtrMap.contains(token);
}

TokenList ParserContext::getManagedTokens()
{
    return managedTokens;
}
