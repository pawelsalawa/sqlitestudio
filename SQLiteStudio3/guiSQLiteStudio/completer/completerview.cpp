#include "completerview.h"
#include "completeritemdelegate.h"
#include "sqleditor.h"
#include <QKeySequence>
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

void CompleterView::keyPressEvent(QKeyEvent* e)
{
    QKeySequence hotkey = GET_SHORTCUTS_CATEGORY(SqlEditor).COMPLETE.get();
    QKeySequence theKey = QKeySequence(e->key() | e->modifiers());
    if (hotkey == theKey)
        return;

    QString txt = e->text();
    if (!txt.isEmpty() && txt[0].isPrint())
    {
        emit textTyped(txt);
        return;
    }

    switch (e->key())
    {
        case Qt::Key_Backspace:
            emit backspace();
            return;
        case Qt::Key_Left:
            emit left();
            return;
        case Qt::Key_Right:
            emit right();
            return;
    }

    QListView::keyPressEvent(e);
}
