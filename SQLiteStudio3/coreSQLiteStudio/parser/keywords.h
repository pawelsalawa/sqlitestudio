#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QStringList>
#include <QHash>

/** @file */

/**
 * @brief Translates keyword into it's Lemon token ID for SQLite 3 dialect.
 * @param str The keyword.
 * @return Lemon generated token ID, or TK3_ID value when the \p str parameter was not recognized as a valid SQLite 3 keyword.
 *
 * This method is used internally by the Lexer.
 * Comparision is done in case insensitive manner.
 */
API_EXPORT int getKeywordId3(const QString& str);

/**
 * @brief Tests whether given string represents a keyword in SQLite dialect.
 * @param str String to test.
 * @return true if the string represents a keyword, or false otherwise.
 *
 * Comparision is done in case insensitive manner.
 */
API_EXPORT bool isKeyword(const QString& str);

/**
 * @brief Tests whether given string represents a "soft" keyword in SQLite dialect.
 * @param str String to test.
 * @return true if the string represents a soft keyword, or false otherwise.
 *
 * "Soft" keywords are like any SQLite keywords, except these can be treated by SQLite as ID too,
 * if it fits the syntax. It means that when SQLite parses a query, it first tries to parse these
 * "soft" keywords are regular keywords and if it fails, it re-tries with treating them as ID.
 *
 * Quote from SQLite's parse.y:
 * "The following directive causes tokens ABORT, AFTER, ASC, etc. to
 * fallback to ID if they will not parse as their original value."
 *
 * Comparision is done in case insensitive manner.
 */
API_EXPORT bool isSoftKeyword(const QString& str);

/**
 * @brief Tests whether given string representing any variation of ROWID.
 * @param str String to test.
 * @return true if the value represents ROWID keyword, or false otherwise.
 *
 * ROWID keywords understood by SQLite are: <tt>ROWID</tt>, <tt>_ROWID_</tt> and <tt>OID</tt>.
 * Comparision is done in case insensitive manner.
 */
API_EXPORT bool isRowIdKeyword(const QString& str);

/**
 * @brief Provides map of SQLite 3 keywords and their Lemon token IDs.
 * @return Keyword-to-Lemon-ID hash map, keywords are uppercase.
 */
API_EXPORT const QHash<QString,int>& getKeywords3();

/**
 * @brief Provides list of keywords representing types of SQL joins.
 * @return Join type keywords.
 *
 * Join type keywords are: <tt>NATURAL</tt>, <tt>LEFT</tt>, <tt>RIGHT</tt>, <tt>OUTER</tt>, <tt>INNER</tt>, <tt>CROSS</tt>.
 *
 * Join type keywords are distinguished from other keywords, because SQLite grammar definitions
 * allow those keywords to be used in contexts other than just join type definition.
 */
API_EXPORT QStringList getJoinKeywords();

/**
 * @brief Tests whether the keyword is one of join type keywords.
 * @param str String to test.
 * @return true if the value represents join type keyword.
 *
 * This method simply tests if given string is on the list returned from getJoinKeywords().
 *
 * Comparision is done in case insensitive manner.
 */
API_EXPORT bool isJoinKeyword(const QString& str);

/**
 * @brief Returns foreign key "match type" keywords.
 * @return Match type keywords.
 *
 * Match type keywords are: <tt>SIMPLE</tt>, <tt>FULL</tt> and <tt>PARTIAL</tt>.
 *
 * Foreign key match type keywords are distinguished from other keywords, because SQLite grammar
 * definitions allow those keywords to be used in contexts other than just foreign key match type definition.
 */
API_EXPORT QStringList getFkMatchKeywords();

API_EXPORT QSet<QString> getIdFallbackKeywords();

/**
 * @brief Tests whether the given value is one of match type keywords.
 * @param str String to test.
 * @return true if the value represents match type keywords, false otherwise.
 *
 * Comparision is done in case insensitive manner.
 */
API_EXPORT bool isFkMatchKeyword(const QString& str);

/**
 * @brief Initializes internal tables of keywords.
 *
 * This has to be (and it is) done at application startup. It defines all internal hash tables
 * and lists with keywords.
 */
API_EXPORT void initKeywords();

/**
 * @brief Provides list of SQLite conflict algorithm keywords.
 *
 * Conflict algorithms keywords are: <tt>ROLLBACK</tt>, <tt>ABORT</tt>, <tt>FAIL</tt>, <tt>IGNORE</tt>, <tt>REPLACE</tt>.
 *
 * Those keywords are used for example on GUI, when user has an "On conflict" algorithm to pick from drop-down list.
 */
API_EXPORT QStringList getConflictAlgorithms();

/**
 * @brief Provides list of accepted Generated Column storage types in SQLite.
 *
 * Generated column storage keywords are: <tt>STORED</tt>, <tt>VIRTUAL</tt>.
 *
 * Those keywords are used for example on GUI, when user has the storage type to pick from drop-down list.
 */
API_EXPORT QStringList getGeneratedColumnTypes();

#endif // KEYWORDS_H
