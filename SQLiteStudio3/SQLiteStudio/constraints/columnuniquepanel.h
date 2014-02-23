#ifndef COLUMNUNIQUEPANEL_H
#define COLUMNUNIQUEPANEL_H

#include "columnuniqueandnotnullpanel.h"

namespace Ui {
    class ColumnUniquePanel;
}

class ColumnUniquePanel : public ColumnUniqueAndNotNullPanel
{
        Q_OBJECT

    public:
        explicit ColumnUniquePanel(QWidget *parent = 0);

    protected:
        void storeType();
};

#endif // COLUMNUNIQUEPANEL_H
