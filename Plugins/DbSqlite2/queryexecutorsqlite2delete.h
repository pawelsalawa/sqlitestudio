#ifndef QUERYEXECUTORSQLITE2DELETE_H
#define QUERYEXECUTORSQLITE2DELETE_H

#include "db/queryexecutorsteps/queryexecutorstep.h"

/**
 * @brief The QueryExecutorSqlite2Delete class
 *
 * From SQLite2 documentation:
 *
 *     Because of this optimization, the change count for "DELETE FROM table" will be zero
 *     regardless of the number of elements that were originally in the table.
 *     To get an accurate count of the number of rows deleted, use "DELETE FROM table WHERE 1" instead.
 *
 * This extra step will add "WHERE 1" if there is no WHERE for DELETE query.
 *
 */
class QueryExecutorSqlite2Delete : public QueryExecutorStep
{
    public:
        bool exec();
};

#endif // QUERYEXECUTORSQLITE2DELETE_H
