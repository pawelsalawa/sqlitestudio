#ifndef EXTTABLEVIEW_H
#define EXTTABLEVIEW_H

#include "guiSQLiteStudio_global.h"
#include <QTableView>

class GUI_API_EXPORT ExtTableView : public QTableView
{
    public:
        explicit ExtTableView(QWidget* parent = nullptr);

    protected:
        void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif // EXTTABLEVIEW_H
