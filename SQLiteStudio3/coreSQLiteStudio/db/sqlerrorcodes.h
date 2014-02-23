#ifndef SQLERRORCODES_H
#define SQLERRORCODES_H

#include "coreSQLiteStudio_global.h"

/**
 * @brief Custom SQL error codes.
 *
 * Those are custom error codes that can be returned by SqlResults::getErrorCode().
 * Usually error codes come from SQLite itself, but some errors can be generated
 * by SQLiteStudio and for those cases this enum lists possible codes.
 *
 * Codes in this enum are not conflicting with error codes returned from SQLite.
 */
struct API_EXPORT SqlErrorCode
{
    enum
    {
        DB_NOT_OPEN = -1000, /**< Database was not open */
        QUERY_EXECUTOR_ERROR = -1001, /**< QueryExecutor error (its sophisticated logic encountered some problem) */
        PARSER_ERROR = -1002, /**< Parser class could not parse the query, because it was either invalid SQL, or bug in the Parser */
        INTERRUPTED = -1003, /**< Query execution was interrupted */
        INVALID_ARGUMENT = -1004, /**< Passed query argument was invalid (out of range, invalid format, etc.) */
        DB_NOT_DEFINED = -1005 /**< Database was not defined */
    };

    /**
     * @brief Tests if given error code means that execution was interrupted.
     * @param errorCode Error code to test.
     * @return true if the code represents interruption, or false otherwise.
     *
     * This method checks both SqlErrorCode::INTERRUPTED and SQLITE_INTERRUPT values,
     * so if the code is either of them, it returns true.
     */
    static bool isInterrupted(int errorCode);
};

#endif // SQLERRORCODES_H
