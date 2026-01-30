#include "multieditorwidget.h"

MultiEditorWidget::MultiEditorWidget(QWidget *parent) :
    QWidget(parent)
{
}

QString MultiEditorWidget::getPreferredFileFilter()
{
    return QString();
}

void MultiEditorWidget::installEventFilter(QObject* filterObj)
{
    QObject::installEventFilter(filterObj);
    for (QWidget* w : getNoScrollWidgets())
        w->installEventFilter(filterObj);
}

void MultiEditorWidget::setTabLabel(const QString& value)
{
    tabLabel = value;
}

QString MultiEditorWidget::getTabLabel()
{
    return tabLabel;
}

bool MultiEditorWidget::isUpToDate() const
{
    return upToDate;
}

void MultiEditorWidget::setUpToDate(bool value)
{
    upToDate = value;
}
