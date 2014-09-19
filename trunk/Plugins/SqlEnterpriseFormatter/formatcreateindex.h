#ifndef FORMATCREATEINDEX_H
#define FORMATCREATEINDEX_H

#include "formatstatement.h"

class SqliteCreateIndex;

class FormatCreateIndex : public FormatStatement
{
    public:
        FormatCreateIndex(SqliteCreateIndex* createIndex);

    protected:
        void formatInternal();

    private:
        SqliteCreateIndex* createIndex;
};

#endif // FORMATCREATEINDEX_H
