#ifndef FORMATCOMMITTRANS_H
#define FORMATCOMMITTRANS_H

#include "formatstatement.h"

class SqliteCommitTrans;

class FormatCommitTrans : public FormatStatement
{
    public:
        FormatCommitTrans(SqliteCommitTrans* ct);

    protected:
        void formatInternal();

    private:
        SqliteCommitTrans* ct = nullptr;
};

#endif // FORMATCOMMITTRANS_H
