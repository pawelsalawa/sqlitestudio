#ifndef COLUMNNOTNULLPANEL_H
#define COLUMNNOTNULLPANEL_H

#include "guiSQLiteStudio_global.h"
#include "columnuniqueandnotnullpanel.h"

class GUI_API_EXPORT ColumnNotNullPanel : public ColumnUniqueAndNotNullPanel
{
        Q_OBJECT
    public:
        explicit ColumnNotNullPanel(QWidget *parent = 0);

    protected:
        void storeType();
};

#endif // COLUMNNOTNULLPANEL_H
