#ifndef FORMATSAVEPOINT_H
#define FORMATSAVEPOINT_H

#include "formatstatement.h"

class SqliteSavepoint;

class FormatSavepoint : public FormatStatement
{
    public:
        FormatSavepoint(SqliteSavepoint* savepoint);

    protected:
        void formatInternal();

    private:
        SqliteSavepoint* savepoint;
};

#endif // FORMATSAVEPOINT_H
