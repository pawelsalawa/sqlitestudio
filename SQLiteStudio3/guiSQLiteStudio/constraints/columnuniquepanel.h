#ifndef COLUMNUNIQUEPANEL_H
#define COLUMNUNIQUEPANEL_H

#include "columnuniqueandnotnullpanel.h"
#include "guiSQLiteStudio_global.h"

namespace Ui {
    class ColumnUniquePanel;
}

class GUI_API_EXPORT ColumnUniquePanel : public ColumnUniqueAndNotNullPanel
{
        Q_OBJECT

    public:
        explicit ColumnUniquePanel(QWidget *parent = 0);

    protected:
        void storeType();
};

#endif // COLUMNUNIQUEPANEL_H
