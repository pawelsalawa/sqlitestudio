#include "tablewidget.h"
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QLabel>

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
    QStringList cols;
    for (int i = 0, total = rowCount(); i < total; ++i)
    {
        if (!item(i, 0)->isSelected())
            continue;

        cols.clear();
        for (int c = 1; c <= 2; c++)
        {
            if (cellWidget(i, c))
            {
                QLabel* l = dynamic_cast<QLabel*>(cellWidget(i, c));
                if (l)
                    cols << l->text();
            }
            else
            {
                cols << item(i, c)->text();
            }
        }
        strings << cols.join(" ");
    }

    QApplication::clipboard()->setText(strings.join("\n"));
}
