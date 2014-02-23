#ifndef SQLERRORRESULTS_H
#define SQLERRORRESULTS_H

#include "sqlresults.h"
#include <QStringList>

/**
 * @brief SqlResults implementation for returning errors.
 *
 * It's very simple implementation of SqlResults, which has hardcoded number of columns and rows (0).
 * It has single constructor which accepts error code and error message, which are later
 * returned from getErrorCode() and getErrorText().
 */
class SqlErrorResults : public SqlResults
{
    public:
        /**
         * @brief Creates error results with given code and message.
         * @param code Error code.
         * @param text Error message.
         */
        SqlErrorResults(int code, const QString &text);

        /**
         * @see SqlResults::next()
         */
        SqlResultsRowPtr next();

        /**
         * @see SqlResults::next()
         */
        QString getErrorText();

        /**
         * @see SqlResults::next()
         */
        int getErrorCode();

        /**
         * @see SqlResults::next()
         */
        QStringList getColumnNames();

        /**
         * @see SqlResults::next()
         */
        int columnCount();

        /**
         * @see SqlResults::next()
         */
        qint64 rowCount();

        /**
         * @see SqlResults::next()
         */
        qint64 rowsAffected();

    private:
        /**
         * @brief Error message passed in constructor.
         */
        QString errText;

        /**
         * @brief errCode Error code passed in constructor.
         */
        int errCode;
};

#endif // SQLERRORRESULTS_H
