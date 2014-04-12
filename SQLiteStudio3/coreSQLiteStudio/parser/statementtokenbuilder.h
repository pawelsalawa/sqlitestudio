#ifndef STATEMENTTOKENBUILDER_H
#define STATEMENTTOKENBUILDER_H

#include "token.h"
#include "ast/sqliteconflictalgo.h"
#include "ast/sqlitesortorder.h"
#include "dialect.h"

class SqliteStatement;

/**
 * @brief Builder producing token list basing on certain inputs.
 *
 * This builder provides several methods to build list of tokens from various input values. It can produce
 * token list for entire AST objects, or it can produce token list for list of names, etc.
 *
 * Token builder is used in SqliteStatement derived classes to rebuild SqliteStatement::tokens basing on the
 * values in their class members.
 *
 * Typical use case:
 * @code
 * TokenList SqliteCreateView::rebuildTokensFromContents()
 * {
 *     StatementTokenBuilder builder;
 *
 *     builder.withKeyword("CREATE").withSpace();
 *     if (tempKw)
 *         builder.withKeyword("TEMP").withSpace();
 *     else if (temporaryKw)
 *         builder.withKeyword("TEMPORARY").withSpace();
 *
 *     builder.withKeyword("VIEW").withSpace();
 *     if (ifNotExists)
 *         builder.withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS").withSpace();
 *
 *     if (dialect == Dialect::Sqlite3 && !database.isNull())
 *         builder.withOther(database, dialect).withOperator(".");
 *
 *     builder.withOther(view, dialect).withSpace().withKeyword("AS").withStatement(select);
 *
 *     return builder.build();
 * }
 * @endcode
 */
class StatementTokenBuilder
{
    public:
        /**
         * @brief Adds keyword token.
         * @param value Value of the keyword token.
         * @return Reference to the builder for the further building.
         *
         * Keyword \p value gets converted to upper case.
         */
        StatementTokenBuilder& withKeyword(const QString& value);

        /**
         * @brief Adds "other" token (some object name, or other word).
         * @param value Value for the token.
         * @return Reference to the builder for the further building.
         *
         * This is used for table names, etc. The \p value is quoted just as passed.
         */
        StatementTokenBuilder& withOther(const QString& value);

        /**
         * @brief Adds "other" token (some object name, or other word).
         * @param value Value for the token.
         * @param dialect Dialect used for wrapping the value.
         * @return Reference to the builder for the further building.
         *
         * The \p value is wrapped with the proper wrapper using wrapObjIfNeeded().
         *
         * @overload
         */
        StatementTokenBuilder& withOther(const QString& value, Dialect dialect);

        /**
         * @brief Adds list of "other" tokens.
         * @param value List of values for tokens.
         * @param dialect Dialect used for wrapping values.
         * @param separator Optional value for separator tokens.
         * @return Reference to the builder for the further building.
         *
         * Given the input \p value, this method produces list of tokens. Additionally it can put extra separator
         * token between all produced tokens using the \p separator value. To skip separator tokens pass
         * an empty string as the separator value.
         */
        StatementTokenBuilder& withOtherList(const QList<QString>& value, Dialect dialect, const QString& separator = ",");

        /**
         * @brief Adds list of "other" tokens.
         * @param value List of values for tokens.
         * @param separator Optional value for separator tokens.
         * @return Reference to the builder for the further building.
         *
         * Works just like the other withOtherList() method, except it doesn't wrap values with wrapObjIfNeeded().
         *
         * @overload
         */
        StatementTokenBuilder& withOtherList(const QList<QString>& value, const QString& separator = ",");

        /**
         * @brief Adds operator token.
         * @param value Value of the operator (";", "+", etc).
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withOperator(const QString& value);

        /**
         * @brief Adds comment token.
         * @param value Comment value, including start/end characters of the comment.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withComment(const QString& value);

        /**
         * @brief Adds decimal number token.
         * @param value Value for the token.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withFloat(double value);

        /**
         * @brief Add integer numer token.
         * @param value Value for the token.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withInteger(int value);

        /**
         * @brief Adds bind parameter token.
         * @param value Name of the bind parameter, including ":" or "@" at the begining.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withBindParam(const QString& value);

        /**
         * @brief Adds left parenthesis token (<tt>"("</tt>).
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withParLeft();

        /**
         * @brief Adds right parenthesis token (<tt>")"</tt>).
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withParRight();

        /**
         * @brief Adds a single whitespace token.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withSpace();

        /**
         * @brief Adds BLOB value token.
         * @param value BLOB value for the token.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withBlob(const QString& value);

        /**
         * @brief Adds string value token.
         * @param value Value for the token.
         * @return Reference to the builder for the further building.
         *
         * The string is wrapped with single quote characters if it's not wrapped yet.
         */
        StatementTokenBuilder& withString(const QString& value);

        /**
         * @brief Adds set of tokens represeting "ON CONFLICT" statement.
         * @param onConflict Conflict resolution algorithm to build for.
         * @return Reference to the builder for the further building.
         *
         * If algorithm is SqliteConflictAlgo::null, no tokens are added.
         */
        StatementTokenBuilder& withConflict(SqliteConflictAlgo onConflict);

        /**
         * @brief Adds space and <tt>"ASC"/"DESC"</tt> token.
         * @param sortOrder Sort order to use.
         * @return Reference to the builder for the further building.
         *
         * If the sort order is SqliteSortOrder::null, no tokens are added.
         */
        StatementTokenBuilder& withSortOrder(SqliteSortOrder sortOrder);

        /**
         * @brief Adds set of tokens representing entire statement.
         * @param stmt Statement to add tokens for.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withStatement(SqliteStatement* stmt);

        /**
         * @brief Adds already defined list of tokens to this builder.
         * @param tokens Tokens to add.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& withTokens(TokenList tokens);

        /**
         * @brief Adds literal value token (integer, decimal, string, BLOB).
         * @param value Value for the token.
         * @return Reference to the builder for the further building.
         *
         * This method tries to convert given \p value to integer,
         * then to double, then it checks if the value has format <tt>X'...'</tt>
         * and if if succeeded at any of those steps, then it adds appropriate
         * token. If none of above succeeded, then the string token is added.
         */
        StatementTokenBuilder& withLiteralValue(const QVariant& value);

        /**
         * @brief Adds tokens representing list of entire statements.
         * @param stmtList List of statements to add tokens for.
         * @param separator Optional separator to be used for separator tokens.
         * @return Reference to the builder for the further building.
         *
         * This method is very similar to withOtherList(), except it works
         * on the entire statements.
         */
        template <class T>
        StatementTokenBuilder& withStatementList(QList<T*> stmtList, const QString& separator = ",")
        {
            bool first = true;
            foreach (T* stmt, stmtList)
            {
                if (!first)
                {
                    if (!separator.isEmpty())
                        withOperator(separator);

                    withSpace();
                }
                withStatement(stmt);
                first = false;
            }
            return *this;
        }

        /**
         * @brief Provides all tokens added so far as a compat token list.
         * @return List of tokens built so far.
         */
        TokenList build() const;

        /**
         * @brief Cleans up all tokens added so far.
         */
        void clear();

    private:
        /**
         * @brief Adds token of given type and value.
         * @param type Type of the token to add.
         * @param value Value for the token to add.
         * @return Reference to the builder for the further building.
         */
        StatementTokenBuilder& with(Token::Type type, const QString& value);

        /**
         * @brief List of tokens added so far.
         */
        TokenList tokens;

        /**
         * @brief Current character position index.
         *
         * This index is used to generate proper values for Token::start and Token::end.
         * Each added token increments this index by the value length.
         */
        int currentIdx = 0;
};

#endif // STATEMENTTOKENBUILDER_H
