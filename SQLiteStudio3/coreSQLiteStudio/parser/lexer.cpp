#include "lexer.h"
#include "keywords.h"
#include "log.h"
#include "lexer_low_lev.h"
#include "sqlite2_parse.h"
#include "sqlite3_parse.h"
#include "common/utils_sql.h"
#include <QString>
#include <QMultiHash>
#include <QDebug>

QHash<Token::Type,QSet<TokenPtr> > Lexer::everyTokenType2;
QHash<Token::Type,QSet<TokenPtr> > Lexer::everyTokenType3;
QHash<Token*,TokenPtr> Lexer::everyTokenTypePtrMap;
TokenPtr Lexer::semicolonTokenSqlite2;
TokenPtr Lexer::semicolonTokenSqlite3;

Lexer::Lexer(Dialect dialect)
    : dialect(dialect), sqlToTokenize(QString::null)
{
}

Lexer::~Lexer()
{
    cleanUp();
}

TokenList Lexer::tokenize(const QString &sql)
{
    TokenList resultList;
    int lgt;
    TokenPtr token;
    QString str = sql;

    quint64 pos = 0;
    for (;str.size() > 0;)
    {
        if (tolerant)
            token = TolerantTokenPtr::create();
        else
            token = TokenPtr::create();

        lgt = lexerGetToken(str, token, dialect == Dialect::Sqlite2 ? 2 : 3, tolerant);
        if (lgt == 0)
            break;

        token->value = str.mid(0, lgt);
        token->start = pos;
        token->end = pos + lgt - 1;

        resultList << token;
        str = str.mid(lgt);
        pos += lgt;
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

    int lgt = lexerGetToken(sqlToTokenize, token, dialect == Dialect::Sqlite2 ? 2 : 3, tolerant);
    if (lgt == 0)
        return TokenPtr();

    token->value = sqlToTokenize.mid(0, lgt);
    token->start = tokenPosition;
    token->end = tokenPosition + lgt - 1;

    sqlToTokenize = sqlToTokenize.mid(lgt);
    tokenPosition += lgt;

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
    QHashIterator<Token::Type,QSet<TokenPtr> > i(
                dialect == Dialect::Sqlite2 ? everyTokenType2 : everyTokenType3
            );
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

TokenPtr Lexer::getSemicolonToken(Dialect dialect)
{
    return (dialect == Dialect::Sqlite3) ? semicolonTokenSqlite3 : semicolonTokenSqlite2;
}

void Lexer::staticInit()
{
    createTokenType(Dialect::Sqlite3, TK3_SPACE, Token::SPACE, " ");
    createTokenType(Dialect::Sqlite3, TK3_COMMENT, Token::COMMENT, "--");
    createTokenType(Dialect::Sqlite3, TK3_MINUS, Token::OPERATOR, "-");
    createTokenType(Dialect::Sqlite3, TK3_SPACE, Token::SPACE, " ");
    createTokenType(Dialect::Sqlite3, TK3_LP, Token::PAR_LEFT, "(");
    createTokenType(Dialect::Sqlite3, TK3_RP, Token::PAR_RIGHT, ")");
    semicolonTokenSqlite3 =
    createTokenType(Dialect::Sqlite3, TK3_SEMI, Token::OPERATOR, ";");
    createTokenType(Dialect::Sqlite3, TK3_PLUS, Token::OPERATOR, "+");
    createTokenType(Dialect::Sqlite3, TK3_STAR, Token::OPERATOR, "*");
    createTokenType(Dialect::Sqlite3, TK3_SLASH, Token::OPERATOR, "/");
    createTokenType(Dialect::Sqlite3, TK3_COMMENT, Token::COMMENT, "/* */");
    createTokenType(Dialect::Sqlite3, TK3_EQ, Token::OPERATOR, "=");
    createTokenType(Dialect::Sqlite3, TK3_EQ, Token::OPERATOR, "==");
    createTokenType(Dialect::Sqlite3, TK3_LE, Token::OPERATOR, "<=");
    createTokenType(Dialect::Sqlite3, TK3_NE, Token::OPERATOR, "<>");
    createTokenType(Dialect::Sqlite3, TK3_NE, Token::OPERATOR, "!=");
    createTokenType(Dialect::Sqlite3, TK3_LSHIFT, Token::OPERATOR, "<<");
    createTokenType(Dialect::Sqlite3, TK3_LT, Token::OPERATOR, "<");
    createTokenType(Dialect::Sqlite3, TK3_GE, Token::OPERATOR, ">=");
    createTokenType(Dialect::Sqlite3, TK3_RSHIFT, Token::OPERATOR, ">>");
    createTokenType(Dialect::Sqlite3, TK3_GT, Token::OPERATOR, ">");
    createTokenType(Dialect::Sqlite3, TK3_BITOR, Token::OPERATOR, "|");
    createTokenType(Dialect::Sqlite3, TK3_CONCAT, Token::OPERATOR, "||");
    createTokenType(Dialect::Sqlite3, TK3_COMMA, Token::OPERATOR, ",");
    createTokenType(Dialect::Sqlite3, TK3_BITAND, Token::OPERATOR, "&");
    createTokenType(Dialect::Sqlite3, TK3_BITNOT, Token::OPERATOR, "~");
    createTokenType(Dialect::Sqlite3, TK3_STRING, Token::STRING, "' '");
    createTokenType(Dialect::Sqlite3, TK3_ID, Token::OTHER, "id");
    createTokenType(Dialect::Sqlite3, TK3_DOT, Token::OPERATOR, ".");
    createTokenType(Dialect::Sqlite3, TK3_INTEGER, Token::INTEGER, "1");
    createTokenType(Dialect::Sqlite3, TK3_FLOAT, Token::FLOAT, "1.0");
    createTokenType(Dialect::Sqlite3, TK3_VARIABLE, Token::BIND_PARAM, "?");
    createTokenType(Dialect::Sqlite3, TK3_BLOB, Token::BLOB, "X'53'");

    // Contextual ID tokens
    createTokenType(Dialect::Sqlite3, TK3_ID_DB, Token::CTX_DATABASE, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_TAB, Token::CTX_TABLE, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_TAB_NEW, Token::CTX_TABLE_NEW, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_COL, Token::CTX_COLUMN, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_COL_NEW, Token::CTX_COLUMN_NEW, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_COL_TYPE, Token::CTX_COLUMN_TYPE, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_COLLATE, Token::CTX_COLLATION, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_FN, Token::CTX_FUNCTION, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_ERR_MSG, Token::CTX_ERROR_MESSAGE, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_IDX, Token::CTX_INDEX, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_IDX_NEW, Token::CTX_INDEX_NEW, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_VIEW, Token::CTX_VIEW, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_VIEW_NEW, Token::CTX_VIEW_NEW, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_JOIN_OPTS, Token::CTX_JOIN_OPTS, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_CONSTR, Token::CTX_CONSTRAINT, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_FK_MATCH, Token::CTX_FK_MATCH, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_TRANS, Token::CTX_TRANSACTION, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_ALIAS, Token::CTX_ALIAS, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_PRAGMA, Token::CTX_PRAGMA, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_TRIG, Token::CTX_TRIGGER, "");
    createTokenType(Dialect::Sqlite3, TK3_ID_TRIG_NEW, Token::CTX_TRIGGER_NEW, "");
    createTokenType(Dialect::Sqlite3, TK3_CTX_ROWID_KW, Token::CTX_ROWID_KW, "ROWID");
    createTokenType(Dialect::Sqlite3, TK3_ID, Token::CTX_OLD_KW, "OLD");
    createTokenType(Dialect::Sqlite3, TK3_ID, Token::CTX_NEW_KW, "NEW");

    QHashIterator<QString,int> i3(getKeywords3());
    while (i3.hasNext())
    {
        i3.next();
        createTokenType(Dialect::Sqlite3, i3.value(), Token::KEYWORD, i3.key());
    }

    //
    // SQLite 2
    //

    createTokenType(Dialect::Sqlite2, TK2_SPACE, Token::SPACE, " ");
    createTokenType(Dialect::Sqlite2, TK2_COMMENT, Token::COMMENT, "--");
    createTokenType(Dialect::Sqlite2, TK2_MINUS, Token::OPERATOR, "-");
    createTokenType(Dialect::Sqlite2, TK2_SPACE, Token::SPACE, " ");
    createTokenType(Dialect::Sqlite2, TK2_LP, Token::PAR_LEFT, "(");
    createTokenType(Dialect::Sqlite2, TK2_RP, Token::PAR_RIGHT, ")");
    semicolonTokenSqlite2 =
    createTokenType(Dialect::Sqlite2, TK2_SEMI, Token::OPERATOR, ";");
    createTokenType(Dialect::Sqlite2, TK2_PLUS, Token::OPERATOR, "+");
    createTokenType(Dialect::Sqlite2, TK2_STAR, Token::OPERATOR, "*");
    createTokenType(Dialect::Sqlite2, TK2_SLASH, Token::OPERATOR, "/");
    createTokenType(Dialect::Sqlite2, TK2_COMMENT, Token::COMMENT, "/* */");
    createTokenType(Dialect::Sqlite2, TK2_EQ, Token::OPERATOR, "=");
    createTokenType(Dialect::Sqlite2, TK2_EQ, Token::OPERATOR, "==");
    createTokenType(Dialect::Sqlite2, TK2_LE, Token::OPERATOR, "<=");
    createTokenType(Dialect::Sqlite2, TK2_NE, Token::OPERATOR, "<>");
    createTokenType(Dialect::Sqlite2, TK2_NE, Token::OPERATOR, "!=");
    createTokenType(Dialect::Sqlite2, TK2_LSHIFT, Token::OPERATOR, "<<");
    createTokenType(Dialect::Sqlite2, TK2_LT, Token::OPERATOR, "<");
    createTokenType(Dialect::Sqlite2, TK2_GE, Token::OPERATOR, ">=");
    createTokenType(Dialect::Sqlite2, TK2_RSHIFT, Token::OPERATOR, ">>");
    createTokenType(Dialect::Sqlite2, TK2_GT, Token::OPERATOR, ">");
    createTokenType(Dialect::Sqlite2, TK2_BITOR, Token::OPERATOR, "|");
    createTokenType(Dialect::Sqlite2, TK2_CONCAT, Token::OPERATOR, "||");
    createTokenType(Dialect::Sqlite2, TK2_COMMA, Token::OPERATOR, ",");
    createTokenType(Dialect::Sqlite2, TK2_BITAND, Token::OPERATOR, "&");
    createTokenType(Dialect::Sqlite2, TK2_BITNOT, Token::OPERATOR, "~");
    createTokenType(Dialect::Sqlite2, TK2_STRING, Token::STRING, "' '");
    createTokenType(Dialect::Sqlite2, TK2_ID, Token::OTHER, "id");
    createTokenType(Dialect::Sqlite2, TK2_DOT, Token::OPERATOR, ".");
    createTokenType(Dialect::Sqlite2, TK2_INTEGER, Token::INTEGER, "1");
    createTokenType(Dialect::Sqlite2, TK2_FLOAT, Token::FLOAT, "1.0");
    createTokenType(Dialect::Sqlite2, TK2_VARIABLE, Token::BIND_PARAM, "?");

    // Contextual ID tokens
    createTokenType(Dialect::Sqlite2, TK2_ID_DB, Token::CTX_DATABASE, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_TAB, Token::CTX_TABLE, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_TAB_NEW, Token::CTX_TABLE_NEW, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_COL, Token::CTX_COLUMN, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_COL_NEW, Token::CTX_COLUMN_NEW, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_COL_TYPE, Token::CTX_COLUMN_TYPE, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_FN, Token::CTX_FUNCTION, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_ERR_MSG, Token::CTX_ERROR_MESSAGE, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_IDX, Token::CTX_INDEX, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_IDX_NEW, Token::CTX_INDEX_NEW, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_VIEW, Token::CTX_VIEW, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_VIEW_NEW, Token::CTX_VIEW_NEW, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_JOIN_OPTS, Token::CTX_JOIN_OPTS, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_CONSTR, Token::CTX_CONSTRAINT, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_FK_MATCH, Token::CTX_FK_MATCH, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_TRANS, Token::CTX_TRANSACTION, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_ALIAS, Token::CTX_ALIAS, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_PRAGMA, Token::CTX_PRAGMA, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_TRIG, Token::CTX_TRIGGER, "");
    createTokenType(Dialect::Sqlite2, TK2_ID_TRIG_NEW, Token::CTX_TRIGGER_NEW, "");
    createTokenType(Dialect::Sqlite2, TK2_ID, Token::CTX_ROWID_KW, "ROWID");
    createTokenType(Dialect::Sqlite2, TK2_ID, Token::CTX_OLD_KW, "OLD");
    createTokenType(Dialect::Sqlite2, TK2_ID, Token::CTX_NEW_KW, "NEW");

    QHashIterator<QString,int> i2(getKeywords2());
    while (i2.hasNext())
    {
        i2.next();
        createTokenType(Dialect::Sqlite2, i2.value(), Token::KEYWORD, i2.key());
    }
}

QString Lexer::detokenize(const TokenList& tokens)
{
    if (tokens.size() == 0)
        return "";

    QString str;
    foreach (TokenPtr token, tokens)
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
            return token->value.isEmpty() ? wrapObjName(token->value, Dialect::Sqlite3, NameWrapper::DOUBLE_QUOTE) : token->value;
        case Token::CTX_ERROR_MESSAGE:
        case Token::STRING:
            return token->value.isEmpty() ? wrapString(token->value) : token->value;
        default:
            break;
    }
    return token->value;
}

TokenList Lexer::tokenize(const QString& sql, Dialect dialect)
{
    Lexer lexer(dialect);
    return lexer.tokenize(sql);
}

TokenPtr Lexer::getEveryTokenTypePtr(Token *token)
{
    if (everyTokenTypePtrMap.contains(token))
        return everyTokenTypePtrMap[token];

    qDebug() << "Queried token not in Lexer::everyTokenTypePtrMap:" << token->toString();
    return TokenPtr();
}

TokenPtr Lexer::createTokenType(Dialect dialect, int lemonType, Token::Type type, const QString &value)
{
    TokenPtr tokenPtr = TokenPtr::create(lemonType, type, value, -100, -100);
    if (dialect == Dialect::Sqlite2)
        everyTokenType2[type] << tokenPtr;
    else
        everyTokenType3[type] << tokenPtr;

    everyTokenTypePtrMap[tokenPtr.data()] = tokenPtr;
    return tokenPtr;
}
