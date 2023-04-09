#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#include <QList>
#include <QString>
#include <QSet>

/**
 * @brief Lexer for SQLite gramma.
 *
 * Lexer (aka tokenizer) splits SQL string into tokens.
 * Tokens can be then used to syntax analysis, or for other purposes.
 *
 * It is useful if you have to modify some entities in the query,
 * such as string, or object name, but you don't want to deal with
 * all escape characters in the name, or other special characters.
 * Lexer packs such entiries into separate tokens and gives them
 * type, so you know what is the token representing.
 */
class API_EXPORT Lexer
{
    public:
        /**
         * @brief Creates lexer.
         */
        Lexer();

        /**
         * @brief Releases resources.
         */
        virtual ~Lexer();

        /**
         * @brief Tokenizes (splits into tokens) given SQL query.
         * @param sql SQL query to tokenize.
         * @return List of tokens produced from tokenizing query.
         */
        TokenList process(const QString& sql);

        /**
         * @brief Stores given SQL query internally for further processing by the lexer.
         * @param sql Query to remember.
         *
         * This method should be followed by calls to getToken().
         */
        void prepare(const QString& sql);

        /**
         * @brief Gets next token from query defined with prepare().
         * @return Token read from the query, or null token if no more tokens are available.
         *
         * Each call to this method generates token for next part of the query, not tokenized yet.
         * Usual flow for this method looks like this:
         * @code
         * QString query = "...";
         * TokenPtr token;
         * lexer.prepare(query);
         * while (token = lexer.getToken())
         * {
         *     // do stuff with the token
         * }
         * @endcode
         */
        TokenPtr getToken();

        /**
         * @brief Clears query stored with prepare().
         */
        void cleanUp();

        /**
         * @brief Enables or disabled tolerant mode.
         * @param enabled If true, then all multi-line and unfinished tokens (strings, comments) will be reported
         * with invalid=true in TolerantToken, but the token itself will have type like it was finished.
         */
        void setTolerantMode(bool enabled);

        /**
         * @brief Provides static sample tokens of all possible types.
         * @return All possible token types.
         * This method uses static set of tokens, so there's no need
         * to delete them outside.
         *
         * It's used by Parser to try every token type as a possible candidate for a next valid token.
         * You should not need to use this method.
         */
        QSet<TokenPtr> getEveryTokenType();

        /**
         * @brief Gets static sample tokens of given types.
         * @param types List of token types to get tokens for. Last element in the list must be Token::INVALID.
         *
         * It's used by Parser to try every token type as a possible candidate for a next valid token.
         * You should not need to use this method.
         *
         * @overload
         */
        QSet<TokenPtr> getEveryTokenType(QSet<Token::Type> types);

        /**
         * @brief Tests whether lexer finished reading all tokens from the query.
         * @return true if there is no more tokens to be read, or false otherwise.
         *
         * This method simply checks whether there's any characters in the query to be tokenized.
         * The query is the one defined with prepare(). Query shrinks with very call to getToken()
         * and once there's no more characters to consume by getToken(), this method will return false.
         *
         * If you call getToken() after isEnd() returned false, the getToken() will return Token::INVALID token.
         */
        bool isEnd() const;

        /**
         * @brief Initializes internal set of static tokens.
         * Initializes internal set of tokens used by getEveryTokenType().
         */
        static void staticInit();

        /**
         * @brief Restores string from token list.
         * @param tokens List of tokens.
         * @return String that was represented by tokens.
         *
         * It simply joins values of all tokens from the list using empty string separator (that is no separator at all).
         */
        static QString detokenize(const TokenList& tokens);

        /**
         * @brief Translates token to string propert representation.
         * @param token Token to translate.
         * @return Translated string.
         *
         * This method applies wrappers where needed (for strings and ids).
         */
        static QString detokenize(const TokenPtr& token);

        /**
         * @brief Tokenizes given SQL query.
         * @param sql SQL query to tokenize.
         * @return List of tokens from tokenizing.
         *
         * This method is a shortcut for:
         * @code
         * Lexer lexer;
         * lexer.tokenize(sql);
         * @endcode
         */
        static TokenList tokenize(const QString& sql);

        /**
         * @brief Translates token pointer into common token shared pointer.
         * @param token Token pointer to translate.
         * @return Shared pointer if found, or null pointer if not found.
         *
         * This method should be used against token pointers extracted from getEveryTokenType() results.
         * Then pointer from any TokenPtr (returned from getEveryTokenType()) is extracted using the
         * QSharedPointer::data(), then this method can be used to return back to the QSharedPointer.
         *
         * As Lexer keeps static internal list of tokens representing token types,
         * it can translate token pointer into shared pointer by comparing them.
         *
         * This method and getEveryTokenType() methods are used strictly by Parser and you should not
         * need to use them.
         */
        static TokenPtr getEveryTokenTypePtr(Token* token);

        /**
         * @brief Provides token representing semicolon in SQLite dialect.
         * @return Token representing semicolon.
         *
         * This is used by Parser to complete the parsed query in case the input query did not end with semicolon.
         */
        static TokenPtr getSemicolonToken();

    private:
        /**
         * @brief Creates token for every token type internal tables.
         * @param lemonType Lemon token ID for this token type.
         * @param type SQLiteStudio token type.
         * @param value Sample value for the token.
         * @return Created token.
         *
         * Every token type internal tables are populated using this method.
         *
         * @see getEveryTokenType()
         */
        static TokenPtr createTokenType(int lemonType, Token::Type type, const QString& value);

        /**
         * @brief Current "tolerant mode" flag.
         *
         * @see setTolerantMode()
         */
        bool tolerant = false;

        /**
         * @brief SQL query to be tokenized with getToken().
         *
         * It's defined with prepare().
         */
        QString sqlToTokenize;

        /**
         * @brief Token produced by the lexer previously.
         *
         * This is used only by the stateful lexer processing (i.e. with getToken())
         */
        TokenPtr prevTokenProcessed;

        /**
         * @brief Current tokenizer position in the sqlToTokenize.
         *
         * This position index is used to track which SQL characters should be tokenized
         * on next call to getToken().
         *
         * It's reset to 0 by prepare() and cleanUp().
         */
        quint64 tokenPosition;

        /**
         * @brief Internal table of every token type for SQLite 3.
         *
         * Internal token type table contains single token per token type, so it can be used to probe the Parser
         * for next valid token candidates.
         */
        static TokenPtr semicolonTokenSqlite3;

        /**
         * @brief Internal table of every token type for SQLite 3.
         *
         * Set of tokens representing all token types, including diversification by values for keywords and operators.
         * It's used by the Parser to probe candidates for next valid token.
         */
        static QHash<Token::Type,QSet<TokenPtr> > everyTokenType3;

        /**
         * @brief Map of every token type pointer to its QSharedPointer from internal tables.
         *
         * This is used by getEveryTokenTypePtr().
         */
        static QHash<Token*,TokenPtr> everyTokenTypePtrMap;
};

#endif // LEXER_H
