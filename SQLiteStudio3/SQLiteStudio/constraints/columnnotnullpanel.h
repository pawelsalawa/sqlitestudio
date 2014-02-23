#ifndef COLUMNNOTNULLPANEL_H
#define COLUMNNOTNULLPANEL_H

#include "columnuniqueandnotnullpanel.h"

class ColumnNotNullPanel : public ColumnUniqueAndNotNullPanel
{
        Q_OBJECT
    public:
        explicit ColumnNotNullPanel(QWidget *parent = 0);

    protected:
        void storeType();
};

#endif // COLUMNNOTNULLPANEL_H
