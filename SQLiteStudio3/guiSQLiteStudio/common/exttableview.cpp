#include "exttableview.h"
#include <QMouseEvent>

ExtTableView::ExtTableView(QWidget* parent) :
    QTableView(parent)
{

}

void ExtTableView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QModelIndex index = indexAt(e->pos());
    emit doubleClicked(index);
}
