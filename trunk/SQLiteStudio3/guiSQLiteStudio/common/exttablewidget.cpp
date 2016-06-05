#include "exttablewidget.h"

#include <QMouseEvent>

ExtTableWidget::ExtTableWidget(QWidget* parent) :
    QTableWidget(parent)
{

}

void ExtTableWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    QModelIndex index = indexAt(e->pos());
    emit doubleClicked(index);
}
