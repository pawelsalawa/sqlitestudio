#include "completerview.h"
#include "completeritemdelegate.h"
#include <QMouseEvent>
#include <QDebug>

CompleterView::CompleterView(QWidget *parent) :
    QListView(parent)
{
    setItemDelegate(new CompleterItemDelegate(this));
}

void CompleterView::selectFirstVisible()
{
    QModelIndex idx;
    for (int i = 0; i < model()->rowCount(); i++)
    {
        if (isRowHidden(i))
            continue;

        idx = model()->index(i, 0, QModelIndex());
        selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
        break;
    }
}

bool CompleterView::hasVisibleItem() const
{
    return countVisibleItem() > 0;
}

int CompleterView::countVisibleItem() const
{
    int cnt = 0;
    for (int i = 0; i < model()->rowCount(); i++)
    {
        if (!isRowHidden(i))
            cnt++;
    }
    return cnt;
}

void CompleterView::focusOutEvent(QFocusEvent* e)
{
    emit focusOut();
    QListView::focusOutEvent(e);
}
