#ifndef PARSERCONTEXT_H
#define PARSERCONTEXT_H

#include "ast/sqlitequery.h"
#include "parser.h"
#include "../dialect.h"

#include <QHash>
#include <QList>
#include <QSet>

class ParserError;

/**
 * @brief Parser context for SQL parsing process
 * This class should not be used outside of @class Parser.
 */
class ParserContext
{
    friend class Parser;

    public:
        /**
         * @brief Releases all internal resources.
         */
        virtual ~ParserContext();

        /**
         * @brief Adds parsed query to collection of parsed queries.
         * @param query Parsed query (AST object).
         *
         * This is called by Lemon parser.
         */
        void addQuery(SqliteQuery* query);

        /**
         * @brief Stores error at given token with given message.
         * @param token Token at which the error occurred.
         * @param text Error message.
         *
         * This is called by Lemon parser.
         */
        void error(TokenPtr token, const QString& text);

        /**
         * @overload
         */
        void error(Token* token, const QString& text);

        /**
         * @brief Stores error with given message.
         * @param text Error message.
         *
         * This method is used to report an error not related to specific token,
         * like when Lemon's stack would get exceeded (which is very unlikely).
         *
         * This is called by Lemon parser.
         *
         * @overload
         */
        void error(const QString& text);

        /**
         * @brief Stores error with most recently parsed token and given message.
         * @param text Error message.
         *
         * Lemon parser calls it when it found out that the error started at the token before.
         *
         * This is just a minor error, so it will be ognored if ignoreMinorErrors is set.
         */
        void minorErrorAfterLastToken(const QString& text);

        /**
         * @brief Marks next token to be parsed with given error message.
         * @param text Error message.
         *
         * Lemon parser calls it when it knows that any next token will be an error.
         *
         * This is just a minor error, so it will be ognored if ignoreMinorErrors is set.
         */
        void minorErrorBeforeNextToken(const QString& text);

        /**
         * @brief Stores error message for most recently parsed token.
         * @param text Error message.
         *
         * Lemon parser calls it when critical error occurred at the most recently parsed token.
         */
        void errorAfterLastToken(const QString& text);

        /**
         * @brief Stores error message for the next token to be parsed.
         * @param text Error message.
         *
         * Lemon parser calls it when critical error is about to happen at any next token.
         */
        void errorBeforeNextToken(const QString& text);

        /**
         * @brief Reports parsing error at given token position.
         * @param text Error message.
         * @param pos Position relative to after the last token. -1 means the last token, -2 the token before it and so on. -1 is default.
         *
         * This method is only useful when we know exactly which token was problematic. If error relates to some already wrapped
         * syntax rule, it may have many tokens and it's hard to tell which token should we blame, but sometimes it can be calculated.
         * Anyway, the token with error is reported by the pos argument. If you don't pass it, it means the error is at last token.
         *
         * Lemon parser uses it for example when there's a statement <tt>"CREATE TABLE ... (...) WITHOUT ROWID"</tt>. The SQLite grammar
         * rule says, that the <tt>"ROWID"</tt> at the end is not necessarily the <tt>ROWID</tt> keyword, but it can be any word,
         * but for now SQLite doesn't understand any other words at that position anyway and returns errors.
         */
        void errorAtToken(const QString& text, int pos = -1);

        /**
         * @brief Flushes pending errors.
         *
         * In case the errorBeforeNextToken() was called and no more tokens were feed to the context, then this method flushes
         * pending error as the error for the last token consumed, but only if minor errors are not ignored.
         * This happens for example for "SELECT " statement, where it's not correct, but it's a minor error, cause user
         * might enter more contents afterwards.
         */
        void flushErrors();

        /**
         * @brief Translates token pointer to it's shared pointer instance.
         * @param token Token pointer to translate.
         * @return QSharedPointer for the token, or null shared pointer in case of failure.
         *
         * This method works basing on internal collection of managed tokens. At each step of parsing, the internal lexer
         * provides token (in form of shared pointer) and that token is then passed to the Lemon parser (as a pure C++ pointer,
         * extracted from shared pointer). The very same token is stored in the internal collection of managed tokens (as a shared pointer).
         * This method allows to get back to the shared pointer.
         *
         * This method is necessary to use shared pointers together with Lemon parser, which works on unions and won't be able to use
         * shared pointers.
         */
        TokenPtr getTokenPtr(Token* token);

        /**
         * @brief Translates list of token pointers to their shared pointer instances.
         * @param tokens Token pointers to translate.
         * @return List of QSharedPointers.
         *
         * This method is just a convenience way to call getTokenPtr() for a list of pointers.
         */
        TokenList getTokenPtrList(const QList<Token*>& tokens);

        /**
         * @brief Adds token to managed list.
         * @param token Token to be added to managed tokens.
         * Tokens managed by context are shared to the Parser, so the API allows to see all parsed tokens.
         * Some tokens might be created outside of Lexer, so this is the central repository for all tokens to be shared.
         */
        void addManagedToken(TokenPtr token);

        /**
         * @brief Tests whether the token is in the collection of tokens managed by this context.
         * @param token Token to test.
         * @return true if the token is managed by this context, or false if not.
         */
        bool isManagedToken(Token* token);

        /**
         * @brief Provides complete list of tokens managed by this context.
         * @return List of tokens.
         */
        TokenList getManagedTokens();

        /**
         * @brief Tests whether there were any critical errors so far during parsing.
         * @return true if there were no critical errors, or false otherwise.
         */
        bool isSuccessful() const;

        /**
         * @brief Provides access to list of queries parsed so far.
         * @return List of parsed AST objects.
         *
         * If there was an error, then queries from the list might be incomplete, which means their data members
         * may still be initialized with their default values. It depends on where the error appeared in the parsed query string.
         */
        const QList<SqliteQueryPtr>& getQueries();

        /**
         * @brief Provides access to all errors occurred so far.
         * @return List of errors.
         */
        const QList<ParserError*>& getErrors();

        QVariant* handleNumberToken(const QString& tokenValue);
        bool isCandidateForMaxNegativeNumber() const;

        /**
         * @brief Flag indicating if the Lemon parser should setup token collections.
         *
         * This setting allows to define whether the Lemon parser should setup token collections for parsed AST objects.
         * In other words, it tells whether the SqliteStatement::tokens and SqliteStatement::tokensMap should be filled.
         *
         * Sometimes it might be worth to disable it to speed up parsig process, but by default it's enabled.
         */
        bool setupTokens = true;

        /**
         * @brief Flag inficating if the Lemon parser should exectute code for the grammar rules.
         *
         * This setting allows to define whether the Lemon parser should execute the code associated with rules.
         * Disabling it will cause no AST objects to be produced, but it can be used to find out syntax errors.
         * If you don't need AST objects (output from parsing), then you can turn this off to speed up Lemon parser.
         *
         * The Parser class for example turns it of when it probes for next valid token candidates. In that case
         * no AST output objects are used, just information whether the next candidate is valid or not.
         */
        bool executeRules = true;

        /**
         * @brief Flag indicating if the Lemon parser should perform "fallback" logic.
         *
         * The "fallback" login in the Lemon parser is used when the input token is one of the keywords and it failed
         * at that step. Then the "fallback" steps in and converts keyword token into the "ID" token, which represents
         * a name of any object in the database (not necessarily existing one). Then the Lemon parser retries with
         * that ID token and if that fails to fulfill the syntax rules too, then the error is reported.
         *
         * This is enabled by default, cause SQLite usually uses that too. It is for example disabled when looking
         * for the next valid token candidate in Parser::getNextTokenCandidates(), cause for that case we need
         * very strict token matching against the syntax.
         */
        bool doFallbacks = true;

        /**
         * @brief Flag indicating if minor errors should be ignored by the Lemon parser.
         *
         * See description of Parser::parse() for details.
         */
        bool ignoreMinorErrors = false;

        /**
         * @brief Dialect used for the parsing.
         *
         * This is used by the Lemon parser in various situations, like for example when it strips the object name
         * from it's wrapping characters ([], "", ``) - that depends on the dialect.
         */
        Dialect dialect;

    private:
        /**
         * @brief Clears all internal containers and deletes error objects.
         */
        void cleanUp();

        /**
         * @brief List of parsed AST objects.
         */
        QList<SqliteQueryPtr> parsedQueries;

        /**
         * @brief Tokens managed by this context.
         */
        TokenList managedTokens;

        /**
         * @brief Mapping from token pointer to it's shared pointer instance.
         */
        QHash<Token*, TokenPtr> tokenPtrMap;

        /**
         * @brief Flag indicating successful or failure parsing.
         *
         * Changed to false when the error was reported.
         */
        bool successful = true;

        /**
         * @brief List of errors reported by Lemon.
         */
        QList<ParserError*> errors;

        /**
         * @brief Flag indicating that the next token should raise an error.
         *
         * This is set by errorBeforeNextToken() and minorErrorBeforeNextToken().
         */
        bool raiseErrorBeforeNextToken = false;

        /**
         * @brief Error to be used for the error at next token.
         *
         * Defined by errorBeforeNextToken() and minorErrorBeforeNextToken().
         */
        QString nextTokenError;

        /**
         * @brief Indicates that the number value is 1 over the max longlong value.
         *
         * Then the positive number is over max longlong by 1, then this is a candidate
         * for max negative longlong value, but this is to be identified in the next parser reduce level.
         *
         * Because of that this flag is set to false by default each time the number is parsed and is set afterwards
         * to true each timethe value is 1 over max longlong. This way if the 'term' rule includes 'minus' token,
         * then it will be properly converted to max negative longlong.
         */
        bool recentNumberIsCandidateForMaxNegative = false;
};

#endif // PARSERCONTEXT_H
