#ifndef ERDTABLEFKPANEL_H
#define ERDTABLEFKPANEL_H

#include "constraints/tableforeignkeypanel.h"

class ErdTableFkPanel : public TableForeignKeyPanel
{
        Q_OBJECT

    public:
        explicit ErdTableFkPanel(QWidget *parent = 0);

    signals:
        void modified();
};

#endif // ERDTABLEFKPANEL_H
