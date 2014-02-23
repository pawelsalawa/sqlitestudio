#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "../dialect.h"
#include "ast/sqlitequery.h"
#include "ast/sqliteexpr.h"

class Lexer;
class ParserContext;
class ParserError;

/**
 * @brief SQL parser.
 *
 * The Parser analyzes given query and produces an Abstract Syntax Tree (AST).
 * The AST is a tree of objects describing parsed query.
 *
 * Typical use case would be:
 * @code
 * Parser parser(db->getDialect());
 * if (parser.parse(queryString))
 * {
 *     QList<SqliteQueryPtr> queries = parser.getQueries();
 *     qDebug() << "number of queries parsed:" << queries.size();
 *     foreach (SqliteQueryPtr query, queries)
 *     {
 *         // do stuff with parsed queries
 *         // ...
 *         if (query.dynamicCast<SqliteSelect>())
 *         {
 *             qDebug() << "it's a select!";
 *         }
 *     }
 * }
 * else
 * {
 *     qDebug() << "Error while parsing:" << parser.getErrorString();
 * }
 * @endcode
 *
 * There's also a convenient parse<T>() method with template argument.
 *
 * There is a getNextTokenCandidates() to ask for all valid (according to syntax
 * rules) token types to be used after given query string,
 *
 * Finally, there is a parseExpr() to parse just a SQLite expression
 * (http://sqlite.org/lang_expr.html).
 *
 * Parser works basing on SQLite grammar defined in sqlite2.y and sqlite3.y files.
 * Since there are 2 completly separate grammar definitions, there are 2 dialects
 * that the parser works with.
 *
 * This is a high-level API to the Lemon Parser, the original SQLite parser.
 */
class API_EXPORT Parser
{
    public:
        /**
         * @brief Creates parser for given SQLite dialect.
         * @param dialect SQLite dialect to use. Can be changed later with setDialect().
         */
        Parser(Dialect dialect);

        /**
         * @brief Releases internal resources.
         */
        virtual ~Parser();

        /**
         * @brief Enables or disables low-level debug messages for this parser.
         * @param enabled true to enable, false to disable debug messages.
         *
         * Enabling this causes detailed debug messages from the Lemon parser
         * to be printed. It is useful if you cannot understand why the parser
         * thinks that the query is incorrect, etc.
         */
        void setLemonDebug(bool enabled);

        /**
         * @brief Changes dialect used by parser.
         * @param dialect Dialect to use.
         */
        void setDialect(Dialect dialect);

        /**
         * @brief Parses given query string.
         * @param sql SQL query string to parse. Can be multiple queries separated with semicolon.
         * @param ignoreMinorErrors If true, then parser will ignore minor errors. Detailed descritpion below.
         * @return true if the query was successfully parsed, or false if not.
         *
         * When the parser encounters syntax error, it stops and returns false. The AST objects (parsed queries)
         * are partially filled with data - as much as it was possible till the error. Errors can be examined
         * with getErrors() or getErrorString().
         *
         * The \p ignoreMinorErrors allows to ignore minor syntax errors. The minor error is the error
         * when for example there's a SELECT query, but no result column was typed yet. Normally this is incorrect
         * query, cause SELECT statement requires at least 1 result column, but we can tell parser to ignore it.
         *
         * The usual case for minor error is when there's a SQLite expression missing at position, where it's expected,
         * or when the expression is incomplete, like <tt>database.table.</tt> (no column name as the last part).
         */
        bool parse(const QString& sql, bool ignoreMinorErrors = false);

        /**
         * @brief Parses SQLite expression.
         * @param sql SQLite expression.
         * @return Parsed object, or null on failure. Parser doesn't own parsed object, you have to take care of deleting it.
         *
         * SQLite expression is any expression that you could type after <tt>"SELECT * FROM WHERE"</tt>, etc.
         * It's syntax is described at: http://sqlite.org/lang_expr.html
         */
        SqliteExpr* parseExpr(const QString& sql);

        /**
         * @brief Parses given query and returns it AST specialized object.
         * @tparam T Type of AST object to parse into.
         * @param query SQL query string to parse.
         * @return Shared pointer to the parsed AST object, or null pointer if the query could not be parsed,
         * or the parsed object was not of the requested type.
         *
         * This is a convenient method to parse string query, pick first parsed query from getQueries()
         * and case it into desired AST object type. If this process fails at any point, the result returned will be
         * a null pointer.
         *
         * Example:
         * @code
         * Parser parser(db->getDialect());
         * SqliteSelectPtr select = parser.parse<SelectPtr>(queryString);
         * if (!select)
         * {
         *     qCritical() << "Could not parse" << queryString << "to a SELECT statement, details:" << parser.getErrorString();
         *     return;
         * }
         * // do stuff with the 'select' object
         * // ...
         * @endcode
         */
        template <class T>
        QSharedPointer<T> parse(const QString& query)
        {
            if (!parse(query) || getQueries().size() == 0)
                return QSharedPointer<T>();

            return getQueries().first().dynamicCast<T>();
        }

        /**
         * @brief Tests what are possible valid candidates for the next token.
         * @param sql Part of the SQL query to check for the next token.
         * @return List of token candidates.
         *
         * This method gets list of all token types from Lexer::getEveryTokenType() and tests which of them does the parser
         * accept for the next token after the given query.
         *
         * You should treat the results of this method as a list of token <b>types</b>, rather than explicit tokens.
         * Each token in the results represents a logical grammar entity. You should look at the Token::type and Token::value,
         * while the Token::value is meaningful only for Token::KEYWORD, or Token::OPERATOR. For other token types, the value
         * is just an example value (like for Token::INTEGER all numbers are valid candidates, not just one returned
         * from this method).
         */
        TokenList getNextTokenCandidates(const QString& sql);

        /**
         * @brief Provides list of queries parsed recently by the parser.
         * @return List of queries.
         *
         * On successful execution this list should contain at least 1 query, unless parsed query
         * was a blank string - in that case this method will return list with no elements.
         *
         * In case of parsing error it's undefined how many elements will be in the list
         * and also how much of the information will be filled in the queries - it depends on where the error appeared.
         */
        const QList<SqliteQueryPtr>& getQueries();

        /**
         * @brief Provides list of errors that occurred during parsing.
         * @return List of errors.
         *
         * Usually there's just one error, but there are cases when there might be more error on the list.
         * That would be for example if you type "!" somewhere in the query where it should not be.
         * Parser can deal with such errors and proceed. Such errors are later reported as failed parsing after all,
         * but parser can continue and provide more data for AST objects (even they will be result of failed parsing process)
         * and find other errors. In such cases, there can be 2, or even more errors on the list.
         */
        const QList<ParserError*>& getErrors();

        /**
         * @brief Provides error message from recent failed parsing process.
         * @return Error message.
         *
         * This is convenient method to get first error getom getErrors() and return message from it.
         */
        QString getErrorString();

        /**
         * @brief Provides list of tokens procudes during parsing process.
         * @return List of tokens.
         *
         * Parser tokenizes query in order to parse it. It stores those tokens, so you can use them and you don't
         * need to put query through the Lexer again (after Parser did it).
         */
        TokenList getParsedTokens();

        /**
         * @brief Tells whether most recent parsing was successful.
         * @return true if parsing was successful, or false otherwise.
         *
         * This method tells result for: parse(), parse<T>(), getNextTokenCandidates() and parseExpr().
         */
        bool isSuccessful() const;

        /**
         * @brief Clears parser state.
         *
         * Clears any parsed queries, stored tokens, errors, etc.
         */
        void reset();

    private:

        /**
         * @brief Does the actual parsing job.
         * @param sql Query to be parsed.
         * @param lookForExpectedToken true if the parsing should be in "look for valid token candidates" mode,
         * or false for regular mode.
         * @return true on success, or false on failure.
         *
         * Both parse() and getNextTokenCandidates() call this method.
         */
        bool parseInternal(const QString &sql, bool lookForExpectedToken);

        /**
         * @brief Probes token types against the current parser state.
         * @param pParser Pointer to Lemon parser.
         *
         * Probes all token types against current state of the parser. After each probe, the result is stored
         * and the parser state is restored to as what it was before the probe.
         *
         * After all tokens were probed, we have the full information on what tokens are welcome
         * at this parser state. This information is stored in the acceptedTokens member.
         */
        void expectedTokenLookup(void *pParser);

        /**
         * @brief Initializes Parser's internals.
         *
         * Creates internal Lexer and ParserContext.
         */
        void init();

        /**
         * @brief Cleans up Parser's resources.
         *
         * Deletes internal Lexer and ParserContext.
         */
        void cleanUp();

        /**
         * @brief Propagates dialect to all AST objects.
         *
         * This is called after successful parsing to set the adequate SQLite dialect
         * in all AST objects.
         */
        void fillSqliteDialect();

        /**
         * @brief Creates Lemon parser.
         * @return Pointer to Lemon parser.
         */
        void* parseAlloc(void *(*mallocProc)(size_t));

        /**
         * @brief Releases memory of the Lemon parser.
         * @param p Pointer to Lemon parser.
         */
        void  parseFree(void *p, void (*freeProc)(void*));

        /**
         * @brief Invokes next step of Lemon parsing process.
         * @param yyp Pointer to the Lemon parser.
         * @param yymajor Lemon token ID (Token::lemonType) of the next token to be parsed.
         * @param yyminor Next Token object to be parsed.
         * @param parserContext Common context object for the parsing process.
         *
         * This method feeds Lemon parser with next token. This is the major input method
         * for parsing the query. It's a bridge between the high-level Parser API
         * and the low-level Lemon parser.
         */
        void  parse(void *yyp, int yymajor, TokenPtr yyminor, ParserContext* parserContext);

        /**
         * @brief Enables low-level parser debug messages.
         * @param stream Stream to write messages to.
         * @param zPrefix Prefix for all messages.
         */
        void  parseTrace(FILE *stream, char *zPrefix);

        /**
         * @brief Copies Lemon parser state.
         * @param other Input parser state.
         * @return Copied parser state.
         */
        void* parseCopyParserState(void* other);

        /**
         * @brief Restores Lemon parser state from saved copy.
         * @param saved Saved copy of Lemon parser state.
         * @param target Parser state to restore from saved copy.
         */
        void  parseRestoreParserState(void* saved, void* target);

        /**
         * @brief Releases memory used for the Lemon parser state copy.
         * @param other Lemon parser state to be freed.
         */
        void  parseFreeSavedState(void* other);

        /**
         * @brief Adds meaningless token into Lemon's parser stack.
         * @param other Lemon parser.
         * @param token Token to be added.
         *
         * This method is used to add spaces and comments to the Lemon's stack.
         */
        void  parseAddToken(void* other, TokenPtr token);

        /**
         * @brief Parser's dialect.
         */
        Dialect dialect;

        /**
         * @brief Flag indicating if the Lemon low-level debug messages are enabled.
         */
        bool debugLemon = false;

        /**
         * @brief Parser's internal Lexer.
         */
        Lexer* lexer = nullptr;

        /**
         * @brief Parser's internal context shared for the all Lemon parsing steps.
         *
         * Context is used as an output from Lemon parser. Lemon parser stores error details, token maps,
         * and others in it.
         *
         * On the other side, Parser class puts configuration into the Context, so Lemon
         * can use it.
         */
        ParserContext* context = nullptr;

        /**
         * @brief List of valid tokens collected by expectedTokenLookup().
         */
        TokenList acceptedTokens;
};

#endif // PARSER_H
