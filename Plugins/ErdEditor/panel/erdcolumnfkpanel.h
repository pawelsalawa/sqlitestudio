#ifndef ERDCOLUMNFKPANEL_H
#define ERDCOLUMNFKPANEL_H

#include "constraints/columnforeignkeypanel.h"

class ErdColumnFkPanel : public ColumnForeignKeyPanel
{
        Q_OBJECT

    public:
        explicit ErdColumnFkPanel(QWidget *parent = 0);

};

#endif // ERDCOLUMNFKPANEL_H
