#include "sqlqueryitemlineedit.h"
#include <QTimer>

SqlQueryItemLineEdit::SqlQueryItemLineEdit(bool shouldSkipInitialSelection, QWidget *parent)
    : QLineEdit{parent}, shouldSkipInitialSelection(shouldSkipInitialSelection)
{
}

void SqlQueryItemLineEdit::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);
    if (shouldSkipInitialSelection)
        QTimer::singleShot(0, this, &QLineEdit::deselect);
}
