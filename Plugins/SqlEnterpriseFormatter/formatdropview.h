#ifndef FORMATDROPVIEW_H
#define FORMATDROPVIEW_H

#include "formatstatement.h"

class SqliteDropView;

class FormatDropView : public FormatStatement
{
    public:
        FormatDropView(SqliteDropView* dropView);

    protected:
        void formatInternal();

    private:
        SqliteDropView* dropView;
};

#endif // FORMATDROPVIEW_H
