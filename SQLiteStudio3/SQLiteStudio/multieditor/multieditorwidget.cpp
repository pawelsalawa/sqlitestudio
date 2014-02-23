#include "multieditorwidget.h"

MultiEditorWidget::MultiEditorWidget(QWidget *parent) :
    QWidget(parent)
{
}

void MultiEditorWidget::installEventFilter(QObject* filterObj)
{
    QObject::installEventFilter(filterObj);
    foreach (QWidget* w, getNoScrollWidgets())
        w->installEventFilter(filterObj);
}

bool MultiEditorWidget::isUpToDate() const
{
    return upToDate;
}

void MultiEditorWidget::setUpToDate(bool value)
{
    upToDate = value;
}
