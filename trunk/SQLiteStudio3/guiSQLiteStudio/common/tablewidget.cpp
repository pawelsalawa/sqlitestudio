#include "tablewidget.h"
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

TableWidget::TableWidget(QWidget *parent) :
    QTableWidget(parent)
{
}

void TableWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy))
    {
        copy();
        return;
    }

    QTableWidget::keyPressEvent(event);
}

void TableWidget::copy()
{
    QStringList strings;
    for (int i = 0; i < rowCount(); ++i)
        if (item(i, 0)->isSelected())
            strings << item(i, 1)->text() + " " + item(i, 2)->text();

    QApplication::clipboard()->setText(strings.join("\n"));
}
