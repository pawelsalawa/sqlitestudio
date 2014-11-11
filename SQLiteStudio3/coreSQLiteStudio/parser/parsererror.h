#ifndef PARSERERROR_H
#define PARSERERROR_H

#include "coreSQLiteStudio_global.h"
#include "parser/token.h"
#include <QString>

/**
 * @brief Class representing error during SQL parsing.
 *
 * It provides error message and position at which the error occurred.
 */
class API_EXPORT ParserError
{
    public:
        /**
         * @brief Creates error for given token and message.
         * @param token Token that the error occurred at.
         * @param text Error message.
         */
        ParserError(TokenPtr token, const QString& text);

        /**
         * @brief Creates error with given range and message.
         * @param start Position where the error starts.
         * @param end Position where the error ends.
         * @param text Error message.
         */
        ParserError(qint64 start, qint64 end, const QString& text);

        /**
         * @brief Creates global error with given message.
         * @param text Error message.
         *
         * Global errors are not related to any token or position.
         */
        explicit ParserError(const QString& text);

        /**
         * @brief Provides error message.
         * @return Error message.
         */
        QString& getMessage();

        /**
         * @brief Provides start position of the error.
         * @return Character position, or -1 if the error is not related to any position (global error).
         */
        qint64 getFrom();

        /**
         * @brief Provides end position of the error.
         * @return Character position, or -1 if the error is not related to any position (global error).
         */
        qint64 getTo();

        /**
         * @brief Serializes error to readable string.
         * @return Start position and error message in form: <tt>"position: message"</tt>.
         */
        QString toString();

    private:
        /**
         * @brief Error message.
         */
        QString message = QString::null;

        /**
         * @brief Error start position.
         */
        qint64 start = -1;

        /**
         * @brief Error end position.
         */
        qint64 end = -1;
};

#endif // PARSERERROR_H
