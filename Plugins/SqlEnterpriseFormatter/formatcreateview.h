#ifndef FORMATCREATEVIEW_H
#define FORMATCREATEVIEW_H

#include "formatstatement.h"

class SqliteCreateView;

class FormatCreateView : public FormatStatement
{
    public:
        FormatCreateView(SqliteCreateView* createView);

    protected:
        void formatInternal();

    private:
        SqliteCreateView* createView;
};

#endif // FORMATCREATEVIEW_H
