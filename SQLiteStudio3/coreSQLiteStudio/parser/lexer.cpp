#include "lexer.h"
#include "keywords.h"
#include "lexer_low_lev.h"
#include "sqlite3_parse.h"
#include "common/utils_sql.h"
#include <QString>
#include <QMultiHash>
#include <QDebug>

QHash<Token::Type,QSet<TokenPtr> > Lexer::everyTokenType3;
QHash<Token*,TokenPtr> Lexer::everyTokenTypePtrMap;
TokenPtr Lexer::semicolonTokenSqlite3;

Lexer::Lexer()
    : sqlToTokenize(QString())
{
}

Lexer::~Lexer()
{
    cleanUp();
}

TokenList Lexer::process(const QString &sql)
{
    TokenList resultList;
    int lgt;
    TokenPtr prevToken;
    TokenPtr token;
    QString str = sql;

    quint64 pos = 0;
    for (;str.size() > 0;)
    {
        if (tolerant)
            token = TolerantTokenPtr::create();
        else
            token = TokenPtr::create();

        lgt = lexerGetToken(str, token, prevToken, 3, tolerant);
        if (lgt == 0)
            break;

        token->value = str.mid(0, lgt);
        token->start = pos;
        token->end = pos + lgt - 1;

        resultList << token;
        str = str.mid(lgt);
        pos += lgt;
        if (!token->isWhitespace())
            prevToken = token;
    }

    return resultList;
}

void Lexer::prepare(const QString &sql)
{
    sqlToTokenize = sql;
    tokenPosition = 0;
}

TokenPtr Lexer::getToken()
{
    if (sqlToTokenize.isEmpty())
        return TokenPtr();

    TokenPtr token;
    if (tolerant)
        token = TolerantTokenPtr::create();
    else
        token = TokenPtr::create();

    int lgt = lexerGetToken(sqlToTokenize, token, prevTokenProcessed, 3, tolerant);
    if (lgt == 0)
        return TokenPtr();

    token->value = sqlToTokenize.mid(0, lgt);
    token->start = tokenPosition;
    token->end = tokenPosition + lgt - 1;

    sqlToTokenize = sqlToTokenize.mid(lgt);
    tokenPosition += lgt;
    if (!token->isWhitespace())
        prevTokenProcessed = token;

    return token;
}

void Lexer::cleanUp()
{
    sqlToTokenize.clear();
    tokenPosition = 0;
}

void Lexer::setTolerantMode(bool enabled)
{
    tolerant = enabled;
}

QSet<TokenPtr> Lexer::getEveryTokenType()
{
    return getEveryTokenType({Token::BIND_PARAM, Token::BLOB, Token::COMMENT, Token::FLOAT,
                             Token::INTEGER, Token::KEYWORD, Token::OPERATOR, Token::OTHER,
                             Token::PAR_LEFT, Token::PAR_RIGHT, Token::SPACE, Token::STRING,
                             Token::INVALID});
}

QSet<TokenPtr> Lexer::getEveryTokenType(QSet<Token::Type> types)
{
    // Process set of types
    QSet<TokenPtr> results;
    QHashIterator<Token::Type,QSet<TokenPtr> > i(everyTokenType3);
    while (i.hasNext())
    {
        i.next();
        if (types.contains(i.key()))
        {
            QSet<TokenPtr> tk = i.value();
            results.unite(tk);
        }
    }
    return results;
}

bool Lexer::isEnd() const
{
    return sqlToTokenize.isEmpty();
}

TokenPtr Lexer::getSemicolonToken()
{
    return semicolonTokenSqlite3;
}

void Lexer::staticInit()
{
    createTokenType(TK3_SPACE, Token::SPACE, " ");
    createTokenType(TK3_COMMENT, Token::COMMENT, "--");
    createTokenType(TK3_MINUS, Token::OPERATOR, "-");
    createTokenType(TK3_SPACE, Token::SPACE, " ");
    createTokenType(TK3_LP, Token::PAR_LEFT, "(");
    createTokenType(TK3_RP, Token::PAR_RIGHT, ")");
    semicolonTokenSqlite3 =
    createTokenType(TK3_SEMI, Token::OPERATOR, ";");
    createTokenType(TK3_PLUS, Token::OPERATOR, "+");
    createTokenType(TK3_STAR, Token::OPERATOR, "*");
    createTokenType(TK3_SLASH, Token::OPERATOR, "/");
    createTokenType(TK3_COMMENT, Token::COMMENT, "/* */");
    createTokenType(TK3_EQ, Token::OPERATOR, "=");
    createTokenType(TK3_EQ, Token::OPERATOR, "==");
    createTokenType(TK3_LE, Token::OPERATOR, "<=");
    createTokenType(TK3_NE, Token::OPERATOR, "<>");
    createTokenType(TK3_NE, Token::OPERATOR, "!=");
    createTokenType(TK3_LSHIFT, Token::OPERATOR, "<<");
    createTokenType(TK3_LT, Token::OPERATOR, "<");
    createTokenType(TK3_GE, Token::OPERATOR, ">=");
    createTokenType(TK3_RSHIFT, Token::OPERATOR, ">>");
    createTokenType(TK3_GT, Token::OPERATOR, ">");
    createTokenType(TK3_BITOR, Token::OPERATOR, "|");
    createTokenType(TK3_CONCAT, Token::OPERATOR, "||");
    createTokenType(TK3_COMMA, Token::OPERATOR, ",");
    createTokenType(TK3_BITAND, Token::OPERATOR, "&");
    createTokenType(TK3_BITNOT, Token::OPERATOR, "~");
    createTokenType(TK3_STRING, Token::STRING, "' '");
    createTokenType(TK3_ID, Token::OTHER, "id");
    createTokenType(TK3_DOT, Token::OPERATOR, ".");
    createTokenType(TK3_INTEGER, Token::INTEGER, "1");
    createTokenType(TK3_FLOAT, Token::FLOAT, "1.0");
    createTokenType(TK3_VARIABLE, Token::BIND_PARAM, "?");
    createTokenType(TK3_BLOB, Token::BLOB, "X'53'");

    // Contextual ID tokens
    createTokenType(TK3_ID_DB, Token::CTX_DATABASE, "");
    createTokenType(TK3_ID_TAB, Token::CTX_TABLE, "");
    createTokenType(TK3_ID_TAB_NEW, Token::CTX_TABLE_NEW, "");
    createTokenType(TK3_ID_COL, Token::CTX_COLUMN, "");
    createTokenType(TK3_ID_COL_NEW, Token::CTX_COLUMN_NEW, "");
    createTokenType(TK3_ID_COL_TYPE, Token::CTX_COLUMN_TYPE, "");
    createTokenType(TK3_ID_COLLATE, Token::CTX_COLLATION, "");
    createTokenType(TK3_ID_FN, Token::CTX_FUNCTION, "");
    createTokenType(TK3_ID_ERR_MSG, Token::CTX_ERROR_MESSAGE, "");
    createTokenType(TK3_ID_IDX, Token::CTX_INDEX, "");
    createTokenType(TK3_ID_IDX_NEW, Token::CTX_INDEX_NEW, "");
    createTokenType(TK3_ID_VIEW, Token::CTX_VIEW, "");
    createTokenType(TK3_ID_VIEW_NEW, Token::CTX_VIEW_NEW, "");
    createTokenType(TK3_ID_JOIN_OPTS, Token::CTX_JOIN_OPTS, "");
    createTokenType(TK3_ID_CONSTR, Token::CTX_CONSTRAINT, "");
    createTokenType(TK3_ID_FK_MATCH, Token::CTX_FK_MATCH, "");
    createTokenType(TK3_ID_TRANS, Token::CTX_TRANSACTION, "");
    createTokenType(TK3_ID_ALIAS, Token::CTX_ALIAS, "");
    createTokenType(TK3_ID_PRAGMA, Token::CTX_PRAGMA, "");
    createTokenType(TK3_ID_TRIG, Token::CTX_TRIGGER, "");
    createTokenType(TK3_ID_TRIG_NEW, Token::CTX_TRIGGER_NEW, "");
    createTokenType(TK3_CTX_ROWID_KW, Token::CTX_ROWID_KW, "ROWID");
    createTokenType(TK3_CTX_STRICT_KW, Token::CTX_STRICT_KW, "STRICT");
    createTokenType(TK3_ID, Token::CTX_OLD_KW, "OLD");
    createTokenType(TK3_ID, Token::CTX_NEW_KW, "NEW");

    QHashIterator<QString,int> i3(getKeywords3());
    while (i3.hasNext())
    {
        i3.next();
        createTokenType(i3.value(), Token::KEYWORD, i3.key());
    }
}

QString Lexer::detokenize(const TokenList& tokens)
{
    if (tokens.size() == 0)
        return "";

    QString str;
    for (TokenPtr token : tokens)
        str += detokenize(token);

    return str;
}

QString Lexer::detokenize(const TokenPtr& token)
{
    switch (token->type) {
        case Token::OTHER:
        case Token::CTX_ALIAS:
        case Token::CTX_COLLATION:
        case Token::CTX_COLUMN:
        case Token::CTX_COLUMN_NEW:
        case Token::CTX_COLUMN_TYPE:
        case Token::CTX_CONSTRAINT:
        case Token::CTX_DATABASE:
        case Token::CTX_INDEX:
        case Token::CTX_INDEX_NEW:
        case Token::CTX_TABLE:
        case Token::CTX_TABLE_NEW:
        case Token::CTX_TRANSACTION:
        case Token::CTX_TRIGGER:
        case Token::CTX_TRIGGER_NEW:
        case Token::CTX_VIEW:
        case Token::CTX_VIEW_NEW:
            return token->value.isEmpty() ? wrapObjName(token->value, NameWrapper::DOUBLE_QUOTE) : token->value;
        case Token::CTX_ERROR_MESSAGE:
        case Token::STRING:
            return token->value.isEmpty() ? wrapString(token->value) : token->value;
        default:
            break;
    }
    return token->value;
}

TokenList Lexer::tokenize(const QString& sql)
{
    Lexer lexer;
    return lexer.process(sql);
}

TokenPtr Lexer::getEveryTokenTypePtr(Token *token)
{
    if (everyTokenTypePtrMap.contains(token))
        return everyTokenTypePtrMap[token];

    qDebug() << "Queried token not in Lexer::everyTokenTypePtrMap:" << token->toString();
    return TokenPtr();
}

TokenPtr Lexer::createTokenType(int lemonType, Token::Type type, const QString &value)
{
    TokenPtr tokenPtr = TokenPtr::create(lemonType, type, value, -100, -100);
    everyTokenType3[type] << tokenPtr;

    everyTokenTypePtrMap[tokenPtr.data()] = tokenPtr;
    return tokenPtr;
}
