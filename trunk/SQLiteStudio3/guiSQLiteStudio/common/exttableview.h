#ifndef EXTTABLEVIEW_H
#define EXTTABLEVIEW_H

#include <QTableView>

class ExtTableView : public QTableView
{
    public:
        explicit ExtTableView(QWidget* parent = nullptr);

    protected:
        void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif // EXTTABLEVIEW_H
