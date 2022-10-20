#ifndef QUERYEXECUTORCOLUMNTYPE_H
#define QUERYEXECUTORCOLUMNTYPE_H

#include "queryexecutorstep.h"
//#include "parser/ast/sqliteselect.h"

class QueryExecutorColumnType : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();

    private:
        QStringList addTypeColumns();
//        SqliteSelect::Core::ResultColumn* createRealTypeOfResCol(const QString& targetCol, const QString& alias);
};

#endif // QUERYEXECUTORCOLUMNTYPE_H
