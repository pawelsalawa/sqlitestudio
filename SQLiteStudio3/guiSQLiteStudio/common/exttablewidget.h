#ifndef EXTTABLEWIDGET_H
#define EXTTABLEWIDGET_H

#include <QTableWidget>

class ExtTableWidget : public QTableWidget
{
    public:
        explicit ExtTableWidget(QWidget* parent = nullptr);

    protected:
        void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif // EXTTABLEWIDGET_H
