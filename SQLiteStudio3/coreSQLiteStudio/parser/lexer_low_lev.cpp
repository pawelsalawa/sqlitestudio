#include "lexer_low_lev.h"
#include "token.h"
#include "keywords.h"
#include "sqlite3_parse.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "common/unused.h"

#include <QByteArray>
#include <QChar>
#include <QDebug>

//
// Low-level lexer routines based on tokenizer from SQLite 3.7.15.2
//

int lexerGetToken(const QString& z, TokenPtr& token, bool tolerant);

bool isIdChar(const QChar& c)
{
    return c.isPrint() && !c.isSpace() && !doesObjectNeedWrapping(c);
}

// From SQLite source code:
/*
** The following three functions are called immediately after the tokenizer
** reads the keywords WINDOW, OVER and FILTER, respectively, to determine
** whether the token should be treated as a keyword or an SQL identifier.
** This cannot be handled by the usual lemon %fallback method, due to
** the ambiguity in some constructions. e.g.
**
**   SELECT sum(x) OVER ...
**
** In the above, "OVER" might be a keyword, or it might be an alias for the
** sum(x) expression. If a "%fallback ID OVER" directive were added to
** grammar, then SQLite would always treat "OVER" as an alias, making it
** impossible to call a window-function without a FILTER clause.
**
** WINDOW is treated as a keyword if:
**
**   * the following token is an identifier, or a keyword that can fallback
**     to being an identifier, and
**   * the token after than one is TK_AS.
**
** OVER is a keyword if:
**
**   * the previous token was TK_RP, and
**   * the next token is either TK_LP or an identifier.
**
** FILTER is a keyword if:
**
**   * the previous token was TK_RP, and
**   * the next token is TK_LP.
*/

int sqlite3ParserFallback(int iToken); // defined in parse.y
int lexerWindowSpecificGetToken(const QString& z, TokenPtr& token, const TokenPtr& prevToken, bool tolerant)
{
    int lgt = 0;
    do
        lgt += lexerGetToken(z.mid(lgt), token, prevToken, tolerant);
    while (token->lemonType == TK3_SPACE);

    if (
        token->lemonType == TK3_ID ||
        token->lemonType == TK3_STRING ||
        token->lemonType == TK3_JOIN_KW ||
        token->lemonType == TK3_WINDOW ||
        token->lemonType == TK3_OVER ||
        sqlite3ParserFallback(token->lemonType) == TK3_ID
    ) {
        token->lemonType = TK3_ID;
        token->type = Token::OTHER;
    }

    return lgt;
}

void lexerHandleWindowKeyword(const QString& z, TokenPtr& token, const TokenPtr& prevToken, bool tolerant)
{
    UNUSED(prevToken);
    TokenPtr firstAfter = TokenPtr::create();
    int lgt = lexerWindowSpecificGetToken(z, firstAfter, token, tolerant);
    if (firstAfter->lemonType != TK3_ID)
    {
        token->lemonType = TK3_ID;
        token->type = Token::OTHER;
        return;
    }

    TokenPtr secondAfter = TokenPtr::create();
    lexerWindowSpecificGetToken(z.mid(lgt), secondAfter, firstAfter, tolerant);
    if (secondAfter->lemonType != TK3_AS)
    {
        token->lemonType = TK3_ID;
        token->type = Token::OTHER;
        return;
    }
}

void lexerHandleOverKeyword(const QString& z, TokenPtr& token, const TokenPtr& prevToken, bool tolerant)
{
    if (prevToken && prevToken->lemonType == TK3_RP) {
        TokenPtr firstAfter = TokenPtr::create();
        lexerWindowSpecificGetToken(z, firstAfter, token, tolerant);
        if (firstAfter->lemonType == TK3_LP || firstAfter->lemonType == TK3_ID)
            return; // remains OVER keyword
    }
    token->lemonType = TK3_ID;
    token->type = Token::OTHER;
}

void lexerHandleFilterKeyword(const QString& z, TokenPtr& token, const TokenPtr& prevToken, bool tolerant)
{
    if (prevToken && prevToken->lemonType == TK3_RP) {
        TokenPtr firstAfter = TokenPtr::create();
        lexerWindowSpecificGetToken(z, firstAfter, token, tolerant);
        if (firstAfter->lemonType == TK3_LP)
            return; // remains FILTER keyword
    }
    token->lemonType = TK3_ID;
    token->type = Token::OTHER;
}

int lexerGetToken(const QString& z, TokenPtr& token, const TokenPtr& prevToken, int sqliteVersion, bool tolerant)
{
    UNUSED(sqliteVersion);

    int lgt = lexerGetToken(z, token, tolerant);
    if (token->lemonType == TK3_WINDOW)
        lexerHandleWindowKeyword(z.mid(lgt), token, prevToken, tolerant);
    else if (token->lemonType == TK3_OVER)
        lexerHandleOverKeyword(z.mid(lgt), token, prevToken, tolerant);
    else if (token->lemonType == TK3_FILTER)
        lexerHandleFilterKeyword(z.mid(lgt), token, prevToken, tolerant);

    return lgt;
}

int lexerGetToken(const QString& z, TokenPtr& token, bool tolerant)
{
    if (tolerant && !token.dynamicCast<TolerantToken>())
    {
        qCritical() << "lexerGetToken() called with tolerant=true, but not a TolerantToken entity!";
        return 0;
    }

    int i;
    QChar c;
    QChar z0 = charAt(z, 0);

    for (;;)
    {
        if (z0.isSpace())
        {
            for(i=1; charAt(z, i).isSpace(); i++) {}
            token->lemonType = TK3_SPACE;
            token->type = Token::SPACE;
            return i;
        }
        if (z0 == '-')
        {
            if (charAt(z, 1) == '-')
            {
                for (i=2; !(c = charAt(z, i)).isNull() && c != '\n'; i++) {}
                token->lemonType = TK3_COMMENT;
                token->type = Token::COMMENT;
                return i;
            }
            else if (charAt(z, 1) == '>')
            {
                token->lemonType = TK3_PTR;
                token->type = Token::OPERATOR;
                return (charAt(z, 2) == '>') ? 3 : 2;
            }
            token->lemonType = TK3_MINUS;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '(')
        {
            token->lemonType = TK3_LP;
            token->type = Token::PAR_LEFT;
            return 1;
        }
        if (z0 == ')')
        {
            token->lemonType = TK3_RP;
            token->type = Token::PAR_RIGHT;
            return 1;
        }
        if (z0 == ';')
        {
            token->lemonType = TK3_SEMI;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '+')
        {
            token->lemonType = TK3_PLUS;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '*')
        {
            token->lemonType = TK3_STAR;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '/')
        {
            if ( charAt(z, 1) != '*' )
            {
                token->lemonType = TK3_SLASH;
                token->type = Token::OPERATOR;
                return 1;
            }

            if ( charAt(z, 2).isNull() )
            {
                token->lemonType = TK3_COMMENT;
                token->type = Token::COMMENT;
                if (tolerant)
                    token.dynamicCast<TolerantToken>()->invalid = true;

                return 2;
            }
            for (i = 3, c = charAt(z, 2); (c != '*' || charAt(z, i) != '/') && !(c = charAt(z, i)).isNull(); i++) {}

            if (tolerant && (c != '*' || charAt(z, i) != '/'))
                token.dynamicCast<TolerantToken>()->invalid = true;

#if QT_VERSION >= 0x050800
            if ( c.unicode() > 0 )
#else
            if ( c > 0 )
#endif
                i++;
            token->lemonType = TK3_COMMENT;
            token->type = Token::COMMENT;
            return i;
        }
        if (z0 == '%')
        {
            token->lemonType = TK3_REM;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '=')
        {
            token->lemonType = TK3_EQ;
            token->type = Token::OPERATOR;
            return 1 + (charAt(z, 1) == '=');
        }
        if (z0 == '<')
        {
            if ( (c = charAt(z, 1)) == '=' )
            {
                token->lemonType = TK3_LE;
                token->type = Token::OPERATOR;
                return 2;
            }
            else if ( c == '>' )
            {
                token->lemonType = TK3_NE;
                token->type = Token::OPERATOR;
                return 2;
            }
            else if( c == '<' )
            {
                token->lemonType = TK3_LSHIFT;
                token->type = Token::OPERATOR;
                return 2;
            }
            else
            {
                token->lemonType = TK3_LT;
                token->type = Token::OPERATOR;
                return 1;
            }
        }
        if (z0 == '>')
        {
            if ( (c = charAt(z, 1)) == '=' )
            {
                token->lemonType = TK3_GE;
                token->type = Token::OPERATOR;
                return 2;
            }
            else if ( c == '>' )
            {
                token->lemonType = TK3_RSHIFT;
                token->type = Token::OPERATOR;
                return 2;
            }
            else
            {
                token->lemonType = TK3_GT;
                token->type = Token::OPERATOR;
                return 1;
            }
        }
        if (z0 == '!')
        {
            if ( charAt(z, 1) != '=' )
            {
                token->lemonType = TK3_ILLEGAL;
                token->type = Token::INVALID;
                return 2;
            }
            else
            {
                token->lemonType = TK3_NE;
                token->type = Token::OPERATOR;
                return 2;
            }
        }
        if (z0 == '|')
        {
            if( charAt(z, 1) != '|' )
            {
                token->lemonType = TK3_BITOR;
                token->type = Token::OPERATOR;
                return 1;
            }
            else
            {
                token->lemonType = TK3_CONCAT;
                token->type = Token::OPERATOR;
                return 2;
            }
        }
        if (z0 == ',')
        {
            token->lemonType = TK3_COMMA;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '&')
        {
            token->lemonType = TK3_BITAND;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '~')
        {
            token->lemonType = TK3_BITNOT;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '`' ||
            z0 == '\'' ||
            z0 == '"')
        {
            QChar delim = z0;
            for (i = 1; !(c = charAt(z, i)).isNull(); i++)
            {
                if ( c == delim )
                {
                    if( charAt(z, i+1) == delim )
                        i++;
                    else
                        break;
                }
            }
            if ( c == '\'' )
            {
                token->lemonType = TK3_STRING;
                token->type = Token::STRING;
                return i+1;
            }
            else if ( !c.isNull() )
            {
                token->lemonType = TK3_ID;
                token->type = Token::OTHER;
                return i+1;
            }
            else if (tolerant)
            {
                if (z0 == '\'')
                {
                    token->lemonType = TK3_STRING;
                    token->type = Token::STRING;
                }
                else
                {
                    token->lemonType = TK3_ID;
                    token->type = Token::OTHER;
                }
                token.dynamicCast<TolerantToken>()->invalid = true;
                return i;
            }
            else
            {
                token->lemonType = TK3_ILLEGAL;
                token->type = Token::INVALID;
                return i;
            }
        }
        if (z0 == '.')
        {
            if( !charAt(z, 1).isDigit() )
            {
                token->lemonType = TK3_DOT;
                token->type = Token::OPERATOR;
                return 1;
            }
            /*
             * If the next character is a digit, this is a floating point
             * number that begins with ".".  Fall thru into the next case
             */
        }
        if (z0.isDigit() || z0 == '.')
        {
            token->lemonType = TK3_INTEGER;
            token->type = Token::INTEGER;
            if (charAt(z, 0) == '0' && (charAt(z, 1) == 'x' || charAt(z, 1) == 'X') && isHex(charAt(z, 2)))
            {
                for (i=3; isHex(charAt(z, i)); i++) {}
                return i;
            }
            for (i=0; charAt(z, i).isDigit(); i++) {}
            if ( charAt(z, i) == '.' )
            {
                i++;
                while ( charAt(z, i).isDigit() )
                    i++;

                token->lemonType = TK3_FLOAT;
                token->type = Token::FLOAT;
            }
            if ( (charAt(z, i) == 'e' || charAt(z, i) == 'E') &&
                 ( charAt(z, i+1).isDigit()
                   || ((charAt(z, i+1) == '+' || charAt(z, i+1) == '-') && charAt(z, i+2).isDigit())
                 )
               )
            {
                i += 2;
                while ( charAt(z, i).isDigit() )
                    i++;

                token->lemonType = TK3_FLOAT;
                token->type = Token::FLOAT;
            }
            while ( isIdChar(charAt(z, i)) )
            {
                token->lemonType = TK3_ILLEGAL;
                token->type = Token::INVALID;
                i++;
            }
            return i;
        }
        if (z0 == '[')
        {
            for (i = 1, c = z0; c!=']' && !(c = charAt(z, i)).isNull(); i++) {}
            if (c == ']')
            {
                token->lemonType = TK3_ID;
                token->type = Token::OTHER;
            }
            else if (tolerant)
            {
                token->lemonType = TK3_ID;
                token->type = Token::OTHER;
                token.dynamicCast<TolerantToken>()->invalid = true;
            }
            else
            {
                token->lemonType = TK3_ILLEGAL;
                token->type = Token::INVALID;
            }
            return i;
        }
        if (z0 == '?')
        {
            token->lemonType = TK3_VARIABLE;
            token->type = Token::BIND_PARAM;
            for (i=1; charAt(z, i).isDigit(); i++) {}
            return i;
        }
        if (z0 == '$' ||
            z0 == '@' ||  /* For compatibility with MS SQL Server */
            z0 == ':')
        {
            int n = 0;
            token->lemonType = TK3_VARIABLE;
            token->type = Token::BIND_PARAM;
            for (i = 1; !(c = charAt(z, i)).isNull(); i++)
            {
                if ( isIdChar(c) )
                {
                    n++;
                }
                else if ( c == '(' && n > 0 )
                {
                    do
                    {
                        i++;
                    }
                    while ( !(c = charAt(z, i)).isNull() && !c.isSpace() && c != ')' );

                    if ( c==')' )
                    {
                        i++;
                    }
                    else
                    {
                        token->lemonType = TK3_ILLEGAL;
                        token->type = Token::INVALID;
                    }
                    break;
                }
                else if ( c == ':' && charAt(z, i+1) == ':' )
                {
                    i++;
                }
                else
                {
                    break;
                }
            }
            if( n == 0 )
            {
                token->lemonType = TK3_ILLEGAL;
                token->type = Token::INVALID;
            }

            return i;
        }
        if (z0 == 'x' || z0 == 'X')
        {
            if ( charAt(z, 1) == '\'' )
            {
                token->lemonType = TK3_BLOB;
                token->type = Token::BLOB;
                for (i = 2; isXDigit(charAt(z, i)); i++) {}
                if (charAt(z, i) != '\'' || i%2)
                {
                    if (tolerant)
                    {
                        token->lemonType = TK3_BLOB;
                        token->type = Token::BLOB;
                        token.dynamicCast<TolerantToken>()->invalid = true;
                    }
                    else
                    {
                        token->lemonType = TK3_ILLEGAL;
                        token->type = Token::INVALID;
                    }
#if QT_VERSION >= 0x050800
                    while (charAt(z, i).unicode() > 0 && charAt(z, i).unicode() != '\'')
#else
                    while (charAt(z, i) > 0 && charAt(z, i) != '\'')
#endif
                        i++;
                }
#if QT_VERSION >= 0x050800
                if ( charAt(z, i).unicode() > 0 )
#else
                if ( charAt(z, i) > 0 )
#endif
                    i++;

                return i;
            }
            /* Otherwise fall through to the next case */
        }
        //default:
        {
            if (!isIdChar(z0))
                break;

            for (i = 1; isIdChar(charAt(z, i)); i++) {}

            token->lemonType = getKeywordId3(z.mid(0, i));

            if (token->lemonType == TK3_ID)
                token->type = Token::OTHER;
            else
                token->type = Token::KEYWORD;

            return i;
        }
    }

    token->lemonType = TK3_ILLEGAL;
    token->type = Token::INVALID;
    return 1;
}

