#ifndef LEXER_LOW_LEV_H
#define LEXER_LOW_LEV_H

#include "parser/token.h"
#include <QString>
#include <QTextStream>

/** @file */

/**
 * @brief Low level tokenizer function used by the Lexer.
 * @param z Query to tokenize.
 * @param[out] token Token container to fill with values. Can be also a TolerantToken.
 * @param prevToken Previous token returned from this function (if this is a subsequent call), or empty pointer othwewise. It's required for a contextual logic.
 * @param sqliteVersion SQLite version, for which the tokenizer should work (currently only 3).
 * Version affects the list of recognized keywords, a BLOB expression and an object name wrapper with the grave accent character (`).
 * @param tolerant If true, then all multi-line and unfinished tokens (strings, comments)
 * will be reported with invalid=true in TolerantToken, but the token itself will have type like it was finished.
 * If this is true, then \p token must be of type TolerantToken, otherwise the the method will return 0 and log a critical error.
 * @return Lemon token ID (see sqlite3_parse.h for possible token IDs).
 *
 * You shouldn't normally need to use this method. Instead of that, use Lexer class, as it provides higher level API.
 *
 * Most of the method code was taken from SQLite tokenizer code. It is modified to support both SQLite 3 gramma
 * and other SQLiteStudio specific features.
 */
int lexerGetToken(const QString& z, TokenPtr& token, const TokenPtr& prevToken, int sqliteVersion, bool tolerant = false);

#endif // LEXER_LOW_LEV_H
