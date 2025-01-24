#ifndef ERDTABLEFKPANEL_H
#define ERDTABLEFKPANEL_H

#include "constraints/tableforeignkeypanel.h"

class ErdTableFkPanel : public TableForeignKeyPanel
{
        Q_OBJECT

    public:
        explicit ErdTableFkPanel(QWidget *parent = 0);
};

#endif // ERDTABLEFKPANEL_H
