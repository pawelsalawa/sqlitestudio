#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include "guiSQLiteStudio_global.h"
#include <QTableWidget>

class QKeyEvent;

class GUI_API_EXPORT TableWidget : public QTableWidget
{
        Q_OBJECT
    public:
        explicit TableWidget(QWidget *parent = 0);

    protected:
        void keyPressEvent(QKeyEvent *event);


    public slots:
        void copy();
};

#endif // TABLEWIDGET_H
