#ifndef FORMATINDEXEDCOLUMN_H
#define FORMATINDEXEDCOLUMN_H

#include "formatstatement.h"

class SqliteIndexedColumn;

class FormatIndexedColumn : public FormatStatement
{
    public:
        FormatIndexedColumn(SqliteIndexedColumn* idxCol);

    protected:
        void formatInternal();

    private:
        SqliteIndexedColumn* idxCol;
};

#endif // FORMATINDEXEDCOLUMN_H
