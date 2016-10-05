#include "lexer_low_lev.h"
#include "token.h"
#include "keywords.h"
#include "sqlite3_parse.h"
#include "sqlite2_parse.h"
#include "common/utils.h"
#include "common/utils_sql.h"

#include <QByteArray>
#include <QChar>
#include <QDebug>

//
// Low-level lexer routines based on tokenizer from SQLite 3.7.15.2
//

bool isIdChar(const QChar& c)
{
    return c.isPrint() && !c.isSpace() && !doesObjectNeedWrapping(c);
}

int lexerGetToken(const QString& z, TokenPtr token, int sqliteVersion, bool tolerant)
{
    if (sqliteVersion < 2 || sqliteVersion > 3)
    {
        qCritical() << "lexerGetToken() called with invalid sqliteVersion:" << sqliteVersion;
        return 0;
    }

    if (tolerant && !token.dynamicCast<TolerantToken>())
    {
        qCritical() << "lexerGetToken() called with tolerant=true, but not a TolerantToken entity!";
        return 0;
    }

    bool v3 = sqliteVersion == 3;
    int i;
    QChar c;
    QChar z0 = charAt(z, 0);

    for (;;)
    {
        if (z0.isSpace())
        {
            for(i=1; charAt(z, i).isSpace(); i++) {}
            token->lemonType = v3 ? TK3_SPACE : TK2_SPACE;
            token->type = Token::SPACE;
            return i;
        }
        if (z0 == '-')
        {
            if (charAt(z, 1) == '-')
            {
                for (i=2; (c = charAt(z, i)) != 0 && c != '\n'; i++) {}
                token->lemonType = v3 ? TK3_COMMENT : TK2_COMMENT;
                token->type = Token::COMMENT;
                return i;
            }
            token->lemonType = v3 ? TK3_MINUS : TK2_MINUS;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '(')
        {
            token->lemonType = v3 ? TK3_LP : TK2_LP;
            token->type = Token::PAR_LEFT;
            return 1;
        }
        if (z0 == ')')
        {
            token->lemonType = v3 ? TK3_RP : TK2_RP;
            token->type = Token::PAR_RIGHT;
            return 1;
        }
        if (z0 == ';')
        {
            token->lemonType = v3 ? TK3_SEMI : TK2_SEMI;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '+')
        {
            token->lemonType = v3 ? TK3_PLUS : TK2_PLUS;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '*')
        {
            token->lemonType = v3 ? TK3_STAR : TK2_STAR;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '/')
        {
            if ( charAt(z, 1) != '*' )
            {
                token->lemonType = v3 ? TK3_SLASH : TK2_SLASH;
                token->type = Token::OPERATOR;
                return 1;
            }

            if ( charAt(z, 2) == 0 )
            {
                token->lemonType = v3 ? TK3_COMMENT : TK2_COMMENT;
                token->type = Token::COMMENT;
                if (tolerant)
                    token.dynamicCast<TolerantToken>()->invalid = true;

                return 2;
            }
            for (i = 3, c = charAt(z, 2); (c != '*' || charAt(z, i) != '/') && (c = charAt(z, i)) != 0; i++) {}

            if (tolerant && (c != '*' || charAt(z, i) != '/'))
                token.dynamicCast<TolerantToken>()->invalid = true;

            if ( c > 0 )
                i++;
            token->lemonType = v3 ? TK3_COMMENT : TK2_COMMENT;
            token->type = Token::COMMENT;
            return i;
        }
        if (z0 == '%')
        {
            token->lemonType = v3 ? TK3_REM : TK2_REM;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '=')
        {
            token->lemonType = v3 ? TK3_EQ : TK2_EQ;
            token->type = Token::OPERATOR;
            return 1 + (charAt(z, 1) == '=');
        }
        if (z0 == '<')
        {
            if ( (c = charAt(z, 1)) == '=' )
            {
                token->lemonType = v3 ? TK3_LE : TK2_LE;
                token->type = Token::OPERATOR;
                return 2;
            }
            else if ( c == '>' )
            {
                token->lemonType = v3 ? TK3_NE : TK2_NE;
                token->type = Token::OPERATOR;
                return 2;
            }
            else if( c == '<' )
            {
                token->lemonType = v3 ? TK3_LSHIFT : TK2_LSHIFT;
                token->type = Token::OPERATOR;
                return 2;
            }
            else
            {
                token->lemonType = v3 ? TK3_LT : TK2_LT;
                token->type = Token::OPERATOR;
                return 1;
            }
        }
        if (z0 == '>')
        {
            if ( (c = charAt(z, 1)) == '=' )
            {
                token->lemonType = v3 ? TK3_GE : TK2_GE;
                token->type = Token::OPERATOR;
                return 2;
            }
            else if ( c == '>' )
            {
                token->lemonType = v3 ? TK3_RSHIFT : TK2_RSHIFT;
                token->type = Token::OPERATOR;
                return 2;
            }
            else
            {
                token->lemonType = v3 ? TK3_GT : TK2_GT;
                token->type = Token::OPERATOR;
                return 1;
            }
        }
        if (z0 == '!')
        {
            if ( charAt(z, 1) != '=' )
            {
                token->lemonType = v3 ? TK3_ILLEGAL : TK2_ILLEGAL;
                token->type = Token::INVALID;
                return 2;
            }
            else
            {
                token->lemonType = v3 ? TK3_NE : TK2_NE;
                token->type = Token::OPERATOR;
                return 2;
            }
        }
        if (z0 == '|')
        {
            if( charAt(z, 1) != '|' )
            {
                token->lemonType = v3 ? TK3_BITOR : TK2_BITOR;
                token->type = Token::OPERATOR;
                return 1;
            }
            else
            {
                token->lemonType = v3 ? TK3_CONCAT : TK2_CONCAT;
                token->type = Token::OPERATOR;
                return 2;
            }
        }
        if (z0 == ',')
        {
            token->lemonType = v3 ? TK3_COMMA : TK2_COMMA;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '&')
        {
            token->lemonType = v3? TK3_BITAND : TK2_BITAND;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '~')
        {
            token->lemonType = v3? TK3_BITNOT : TK2_BITAND;
            token->type = Token::OPERATOR;
            return 1;
        }
        if (z0 == '`' ||
            z0 == '\'' ||
            z0 == '"')
        {
            QChar delim = z0;
            for (i = 1; (c = charAt(z, i)) != 0; i++)
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
                token->lemonType = v3? TK3_STRING : TK2_STRING;
                token->type = Token::STRING;
                return i+1;
            }
            else if ( c != 0 )
            {
                token->lemonType = v3 ? TK3_ID : TK2_ID;
                token->type = Token::OTHER;
                return i+1;
            }
            else if (tolerant)
            {
                if (z0 == '\'')
                {
                    token->lemonType = v3 ? TK3_STRING : TK2_STRING;
                    token->type = Token::STRING;
                }
                else
                {
                    token->lemonType = v3 ? TK3_ID : TK2_ID;
                    token->type = Token::OTHER;
                }
                token.dynamicCast<TolerantToken>()->invalid = true;
                return i;
            }
            else
            {
                token->lemonType = v3 ? TK3_ILLEGAL : TK2_ILLEGAL;
                token->type = Token::INVALID;
                return i;
            }
        }
        if (z0 == '.')
        {
            if( !charAt(z, 1).isDigit() )
            {
                token->lemonType = v3 ? TK3_DOT : TK2_DOT;
                token->type = Token::OPERATOR;
                return 1;
            }
            /*
             * If the next character is a digit, this is a floating point
             * number that begins with ".".  Fall thru into the next case
             */
        }
        if (z0.isDigit())
        {
            token->lemonType = v3 ? TK3_INTEGER : TK2_INTEGER;
            token->type = Token::INTEGER;
            if (v3 && charAt(z, 0) == '0' && (charAt(z, 1) == 'x' || charAt(z, 1) == 'X') && isHex(charAt(z, 2)))
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

                token->lemonType = v3 ? TK3_FLOAT : TK2_FLOAT;
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

                token->lemonType = v3 ? TK3_FLOAT : TK2_FLOAT;
                token->type = Token::FLOAT;
            }
            while ( isIdChar(charAt(z, i)) )
            {
                token->lemonType = v3 ? TK3_ILLEGAL : TK2_ILLEGAL;
                token->type = Token::INVALID;
                i++;
            }
            return i;
        }
        if (z0 == '[')
        {
            for (i = 1, c = z0; c!=']' && (c = charAt(z, i)) != 0; i++) {}
            if (c == ']')
            {
                token->lemonType = v3 ? TK3_ID : TK2_ID;
                token->type = Token::OTHER;
            }
            else if (tolerant)
            {
                token->lemonType = v3 ? TK3_ID : TK2_ID;
                token->type = Token::OTHER;
                token.dynamicCast<TolerantToken>()->invalid = true;
            }
            else
            {
                token->lemonType = v3 ? TK3_ILLEGAL : TK2_ILLEGAL;
                token->type = Token::INVALID;
            }
            return i;
        }
        if (z0 == '?')
        {
            token->lemonType = v3 ? TK3_VARIABLE : TK2_VARIABLE;
            token->type = Token::BIND_PARAM;
            for (i=1; charAt(z, i+2).isDigit(); i++) {}
            return i;
        }
        if (z0 == '$' ||
            z0 == '@' ||  /* For compatibility with MS SQL Server */
            z0 == ':')
        {
            int n = 0;
            token->lemonType = v3 ? TK3_VARIABLE : TK2_VARIABLE;
            token->type = Token::BIND_PARAM;
            for (i = 1; (c = charAt(z, i)) != 0; i++)
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
                    while ( (c = charAt(z, i)) != 0 && !c.isSpace() && c != ')' );

                    if ( c==')' )
                    {
                        i++;
                    }
                    else
                    {
                        token->lemonType = v3 ? TK3_ILLEGAL : TK2_ILLEGAL;
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
                token->lemonType = v3 ? TK3_ILLEGAL : TK2_ILLEGAL;
                token->type = Token::INVALID;
            }

            return i;
        }
        if ((z0 == 'x' ||
             z0 == 'X') &&
             v3)
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
                    while (charAt(z, i) > 0 && charAt(z, i) != '\'')
                        i++;
                }
                if ( charAt(z, i) > 0 )
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

            if (v3)
                token->lemonType = getKeywordId3(z.mid(0, i));
            else
                token->lemonType = getKeywordId2(z.mid(0, i));

            if (token->lemonType == TK3_ID || token->lemonType == TK2_ID)
                token->type = Token::OTHER;
            else
                token->type = Token::KEYWORD;

            return i;
        }
    }

    if (v3)
        token->lemonType = TK3_ILLEGAL;
    else
        token->lemonType = TK2_ILLEGAL;

    token->type = Token::INVALID;
    return 1;
}

