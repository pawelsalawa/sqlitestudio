#ifndef TABLEUNIQUEPANEL_H
#define TABLEUNIQUEPANEL_H

#include "tablepkanduniquepanel.h"

class TableUniquePanel : public TablePrimaryKeyAndUniquePanel
{
        Q_OBJECT
    public:
        explicit TableUniquePanel(QWidget *parent = 0);

        void storeConfiguration();
};

#endif // TABLEUNIQUEPANEL_H
