#ifndef FORMATDROPINDEX_H
#define FORMATDROPINDEX_H

#include "formatstatement.h"

class SqliteDropIndex;

class FormatDropIndex : public FormatStatement
{
    public:
        FormatDropIndex(SqliteDropIndex* dropIndex);

    protected:
        void formatInternal();

    private:
        SqliteDropIndex* dropIndex;
};

#endif // FORMATDROPINDEX_H
