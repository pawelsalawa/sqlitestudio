#ifndef QUERYEXECUTORDATASOURCES_H
#define QUERYEXECUTORDATASOURCES_H

#include "queryexecutorstep.h"

/**
 * @brief Finds all source tables.
 *
 * Finds all source tables for the SELECT query (if it's the last query in the query string)
 * and stores them in QueryExecutor::Context::sourceTables. They are later provided by QueryExecutor::getSourceTables()
 * as a meta information about data sources.
 *
 * Source tables are tables that result columns come from. If there's multiple columns selected
 * from single table, only single table is resolved.
 */
class QueryExecutorDataSources : public QueryExecutorStep
{
        Q_OBJECT
    public:
        bool exec();

};

#endif // QUERYEXECUTORDATASOURCES_H
