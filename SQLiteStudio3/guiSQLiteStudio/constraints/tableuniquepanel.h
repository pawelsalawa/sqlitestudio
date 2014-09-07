#ifndef TABLEUNIQUEPANEL_H
#define TABLEUNIQUEPANEL_H

#include "tablepkanduniquepanel.h"
#include "guiSQLiteStudio_global.h"

class GUI_API_EXPORT TableUniquePanel : public TablePrimaryKeyAndUniquePanel
{
        Q_OBJECT
    public:
        explicit TableUniquePanel(QWidget *parent = 0);

        void storeConfiguration();
};

#endif // TABLEUNIQUEPANEL_H
