#ifndef FORMATUPSERT_H
#define FORMATUPSERT_H

#include "formatstatement.h"

class SqliteUpsert;

class FormatUpsert : public FormatStatement
{
    public:
        FormatUpsert(SqliteUpsert* upsert);

        void formatInternal();

    private:
        SqliteUpsert* upsert = nullptr;
};

#endif // FORMATUPSERT_H
