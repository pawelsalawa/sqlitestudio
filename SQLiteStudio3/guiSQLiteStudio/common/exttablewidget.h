#ifndef EXTTABLEWIDGET_H
#define EXTTABLEWIDGET_H

#include "guiSQLiteStudio_global.h"
#include <QTableWidget>

class GUI_API_EXPORT ExtTableWidget : public QTableWidget
{
    public:
        explicit ExtTableWidget(QWidget* parent = nullptr);

    protected:
        void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif // EXTTABLEWIDGET_H
