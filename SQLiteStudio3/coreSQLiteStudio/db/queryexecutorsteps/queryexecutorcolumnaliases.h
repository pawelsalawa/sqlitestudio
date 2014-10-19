#ifndef QUERYEXECUTORCOLUMNALIASES_H
#define QUERYEXECUTORCOLUMNALIASES_H

#include "queryexecutorstep.h"

/**
 * @brief Handles all column occurrences with proper alias name.
 *
 * After QueryExecutorColumns has assigned new alias names for every result column,
 * this step propagates those new aliases to whole query, so aliased names are used in WHERE clause, ORDER BY clause, etc.
 *
 * This step affects only those columns, that were aliased by user in original query, because unaliased columns will be refereed
 * by their original name, so it's okay. User-aliased columns are problematic, because we changed their alias name to other alias name
 * and now any occurrence of user-defined alias is unknown to the query.
 */
class QueryExecutorColumnAliases : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORCOLUMNALIASES_H
