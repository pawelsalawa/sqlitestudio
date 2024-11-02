#ifndef QUERYEXECUTORSMARTHINTS_H
#define QUERYEXECUTORSMARTHINTS_H

#include "queryexecutorstep.h"

class SqliteColumnType;

/**
 * @brief Analyzes executed query and gives useful hints to user.
 *
 * Originally this class was introduced to address the #4774 issue.
 *
 * Intention of this step is to find any useful hints that may be provided to user in the notification area.
 */
class QueryExecutorSmartHints : public QueryExecutorStep
{
    Q_OBJECT

    public:
        bool exec();

    private:
        void checkForFkDataTypeMismatch(const SqliteQueryPtr& query);
        void checkForFkDataTypeMismatch(const QString& localTable, const QString& localColumn, const DataType& localType,
                                        const QString& fkTable, const QString& fkColumn, const DataType& fkType);
};

#endif // QUERYEXECUTORSMARTHINTS_H
