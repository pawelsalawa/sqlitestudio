#include "sqlqueryitemlineedit.h"

SqlQueryItemLineEdit::SqlQueryItemLineEdit(bool shouldSkipInitialSelection, QWidget *parent)
    : QLineEdit{parent}, shouldSkipInitialSelection(shouldSkipInitialSelection)
{

}

void SqlQueryItemLineEdit::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);
    if (shouldSkipInitialSelection)
        deselect();
}
