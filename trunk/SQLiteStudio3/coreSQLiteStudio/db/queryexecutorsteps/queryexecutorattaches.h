#ifndef QUERYEXECUTORATTACHES_H
#define QUERYEXECUTORATTACHES_H

#include "queryexecutorstep.h"
#include "parser/token.h"
#include <QObject>

/**
 * @brief Checks for any databases required to attach and attaches them.
 *
 * If the query contains any name that is identified to be name of database registered in DbManager,
 * then that database gets attached to the current database (the one that we execute query on)
 * and its attach name is stored in the query executor context, so all attached databases
 * can be later detached.
 *
 * This step accomplishes a transparent database attaching feature of SQLiteStudio.
 *
 * @see DbAttacher
 */
class QueryExecutorAttaches : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORATTACHES_H
