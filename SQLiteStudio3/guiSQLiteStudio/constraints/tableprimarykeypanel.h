#ifndef PRIMARYKEYPANEL_H
#define PRIMARYKEYPANEL_H

#include "tablepkanduniquepanel.h"
#include "guiSQLiteStudio_global.h"

class GUI_API_EXPORT TablePrimaryKeyPanel : public TablePrimaryKeyAndUniquePanel
{
        Q_OBJECT

    public:
        explicit TablePrimaryKeyPanel(QWidget *parent = 0);

        void storeConfiguration();

    private:
        void readConstraint();

    private slots:
        void updateState();
};

#endif // PRIMARYKEYPANEL_H
