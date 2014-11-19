#include "parser.h"
#include "parsercontext.h"
#include "parsererror.h"
#include "lexer.h"
#include "../db/db.h"
#include "ast/sqliteselect.h"
#include <QStringList>
#include <QDebug>

// Generated in sqlite*_parse.c by lemon,
// but not exported in any header
void* sqlite3_parseAlloc(void *(*mallocProc)(size_t));
void  sqlite3_parseFree(void *p, void (*freeProc)(void*));
void  sqlite3_parse(void *yyp, int yymajor, Token* yyminor, ParserContext* parserContext);
void  sqlite3_parseTrace(FILE *stream, char *zPrefix);
void* sqlite3_parseCopyParserState(void* other);
void  sqlite3_parseRestoreParserState(void* saved, void* target);
void  sqlite3_parseFreeSavedState(void* other);
void  sqlite3_parseAddToken(void* other, Token* token);

void* sqlite2_parseAlloc(void *(*mallocProc)(size_t));
void  sqlite2_parseFree(void *p, void (*freeProc)(void*));
void  sqlite2_parse(void *yyp, int yymajor, Token* yyminor, ParserContext* parserContext);
void  sqlite2_parseTrace(FILE *stream, char *zPrefix);
void* sqlite2_parseCopyParserState(void* other);
void  sqlite2_parseRestoreParserState(void* saved, void* target);
void  sqlite2_parseFreeSavedState(void* other);
void  sqlite2_parseAddToken(void* other, Token* token);

Parser::Parser(Dialect dialect)
{
    this->dialect = dialect;
    init();
}

Parser::~Parser()
{
    cleanUp();
}

void Parser::cleanUp()
{
    if (lexer)
    {
        delete lexer;
        lexer = nullptr;
    }

    if (context)
    {
        delete context;
        context = nullptr;
    }
}

void Parser::fillSqliteDialect()
{
    foreach (SqliteQueryPtr query, context->parsedQueries)
        query->setSqliteDialect(dialect);
}

void *Parser::parseAlloc(void *(*mallocProc)(size_t))
{
    if (dialect == Dialect::Sqlite2)
        return sqlite2_parseAlloc(mallocProc);
    else
        return sqlite3_parseAlloc(mallocProc);
}

void Parser::parseFree(void *p, void (*freeProc)(void *))
{
    if (dialect == Dialect::Sqlite2)
        sqlite2_parseFree(p, freeProc);
    else
        sqlite3_parseFree(p, freeProc);
}

void Parser::parse(void *yyp, int yymajor, TokenPtr yyminor, ParserContext *parserContext)
{
    if (dialect == Dialect::Sqlite2)
        sqlite2_parse(yyp, yymajor, yyminor.data(), parserContext);
    else
        sqlite3_parse(yyp, yymajor, yyminor.data(), parserContext);
}

void Parser::parseTrace(FILE *stream, char *zPrefix)
{
    if (dialect == Dialect::Sqlite2)
        sqlite2_parseTrace(stream, zPrefix);
    else
        sqlite3_parseTrace(stream, zPrefix);
}

void *Parser::parseCopyParserState(void *other)
{
    if (dialect == Dialect::Sqlite2)
        return sqlite2_parseCopyParserState(other);
    else
        return sqlite3_parseCopyParserState(other);
}

void Parser::parseRestoreParserState(void *saved, void *target)
{
    if (dialect == Dialect::Sqlite2)
        sqlite2_parseRestoreParserState(saved, target);
    else
        sqlite3_parseRestoreParserState(saved, target);
}

void Parser::parseFreeSavedState(void *other)
{
    if (dialect == Dialect::Sqlite2)
        sqlite2_parseFreeSavedState(other);
    else
        sqlite3_parseFreeSavedState(other);
}

void Parser::parseAddToken(void *other, TokenPtr token)
{
    if (dialect == Dialect::Sqlite2)
        sqlite2_parseAddToken(other, token.data());
    else
        sqlite3_parseAddToken(other, token.data());
}

bool Parser::parse(const QString &sql, bool ignoreMinorErrors)
{
    context->ignoreMinorErrors = ignoreMinorErrors;
    return parseInternal(sql, false);
}

bool Parser::parseInternal(const QString &sql, bool lookForExpectedToken)
{
    void* pParser = parseAlloc( malloc );
    if (debugLemon)
    {
        char* label = nullptr;
        if (dialect == Dialect::Sqlite2)
            label = const_cast<char*>("[LEMON2]: ");
        else
            label = const_cast<char*>("[LEMON3]: ");

        parseTrace(stderr, label);
    }

    reset();
    lexer->prepare(sql);
    context->setupTokens = !lookForExpectedToken;
    context->executeRules = !lookForExpectedToken;
    context->doFallbacks = !lookForExpectedToken;
    context->dialect = dialect;

    TokenPtr token = lexer->getToken();
    if (!token.isNull())
        context->addManagedToken(token);

    bool endsWithSemicolon = false;

    while (token)
    {
        if (token->type == Token::SPACE ||
            token->type == Token::COMMENT ||
            token->type == Token::INVALID)
        {
            parseAddToken(pParser, token);
            token = lexer->getToken();
            if (token)
                context->addManagedToken(token);

            continue;
        }

        endsWithSemicolon = (token->type == Token::OPERATOR && token->value == ";");

        parse(pParser, token->lemonType, token, context);
        token = lexer->getToken();
        if (!token.isNull())
            context->addManagedToken(token);
    }

    if (lookForExpectedToken)
    {
        expectedTokenLookup(pParser);
    }
    else
    {
        if (!endsWithSemicolon)
        {
            token = Lexer::getSemicolonToken(dialect);
            parse(pParser, token->lemonType, token, context);
        }

        qint64 endIdx = sql.length();
        TokenPtr endToken = TokenPtr::create(0, Token::INVALID, QString::null, endIdx, endIdx);
        parse(pParser, 0, endToken, context);
    }

    fillSqliteDialect();

    // Free all non-termials having destructors
    parseFree(pParser, free);

    context->flushErrors();

    if (context->isSuccessful())
    {
        for (SqliteQueryPtr query : context->parsedQueries)
            query->processPostParsing();
    }

    return context->isSuccessful();
}

TokenList Parser::getNextTokenCandidates(const QString &sql)
{
    context->ignoreMinorErrors = true;
    parseInternal(sql, true);
    TokenList results = acceptedTokens;
    acceptedTokens.clear();
    return results;
}

bool Parser::isSuccessful() const
{
    return context->isSuccessful();
}

void Parser::reset()
{
    acceptedTokens.clear();
    lexer->cleanUp();
    context->cleanUp();
}

SqliteExpr *Parser::parseExpr(const QString &sql)
{
    SqliteSelectPtr select = parse<SqliteSelect>("SELECT "+sql+";");
    if (!select || select->coreSelects.size() == 0 || select->coreSelects.first()->resultColumns.size() == 0)
        return nullptr;

    SqliteExpr* expr = select->coreSelects.first()->resultColumns.first()->expr;
    expr->setParent(nullptr);
    return expr;
}

void Parser::expectedTokenLookup(void* pParser)
{
    void* savedParser = parseCopyParserState(pParser);

    ParserContext tempContext;
    tempContext.executeRules = false;
    tempContext.executeRules = false;
    tempContext.doFallbacks = false;
    QSet<TokenPtr> tokenSet =
            lexer->getEveryTokenType({
                Token::KEYWORD, Token::OTHER, Token::PAR_LEFT, Token::PAR_RIGHT, Token::OPERATOR,
                Token::CTX_COLLATION, Token::CTX_COLUMN, Token::CTX_DATABASE, Token::CTX_FUNCTION,
                Token::CTX_INDEX, Token::CTX_JOIN_OPTS, Token::CTX_TABLE, Token::CTX_TRIGGER,
                Token::CTX_VIEW, Token::CTX_FK_MATCH, Token::CTX_ERROR_MESSAGE, Token::CTX_PRAGMA,
                Token::CTX_ALIAS, Token::CTX_TABLE_NEW, Token::CTX_INDEX_NEW, Token::CTX_TRIGGER_NEW,
                Token::CTX_VIEW_NEW, Token::CTX_COLUMN_NEW, Token::CTX_TRANSACTION,
                Token::CTX_CONSTRAINT, Token::CTX_COLUMN_TYPE, Token::CTX_OLD_KW, Token::CTX_NEW_KW,
                Token::CTX_ROWID_KW, Token::INVALID
            });

    foreach (TokenPtr token, tokenSet)
    {
        parse(pParser, token->lemonType, token, &tempContext);

        if (tempContext.isSuccessful())
            acceptedTokens += token;

        tempContext.cleanUp();
        parseRestoreParserState(savedParser, pParser);
    }
    parseFreeSavedState(savedParser);
}

void Parser::init()
{
    lexer = new Lexer(dialect);
    context = new ParserContext();
}

const QList<ParserError *> &Parser::getErrors()
{
    return context->getErrors();
}

QString Parser::getErrorString()
{
    QStringList msgs;
    foreach (ParserError* error, getErrors())
    {
        msgs += error->getMessage();
    }
    return msgs.join(",\n");
}

TokenList Parser::getParsedTokens()
{
    return context->getManagedTokens();
}

void Parser::setLemonDebug(bool enabled)
{
    debugLemon = enabled;
}

void Parser::setDialect(Dialect dialect)
{
    if (this->dialect == dialect)
        return;

    this->dialect = dialect;
    delete lexer;
    lexer = new Lexer(dialect);
}

const QList<SqliteQueryPtr>& Parser::getQueries()
{
    return context->getQueries();
}
