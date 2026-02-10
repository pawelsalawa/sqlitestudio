#include "customsplitter.h"

CustomSplitter::CustomSplitter(QWidget* parent) : QSplitter(parent)
{
    init();
}

CustomSplitter::CustomSplitter(Qt::Orientation orientation, QWidget *parent) : QSplitter(orientation, parent)
{
    init();
}

void CustomSplitter::onSplitterMoved(int pos, int index)
{
    Q_Q_UNUSED(pos);
    Q_Q_UNUSED(index);

    bool anyCollapsed = false;
    for (int i = 0; i < count(); ++i)
    {
        if (sizes().at(i) == 0)
        {
            anyCollapsed = true;
            break;
        }
    }

    if (anyCollapsed)
        setHandleWidth(16);
    else
        setHandleWidth(4);
}

void CustomSplitter::init()
{
    connect(this, &QSplitter::splitterMoved, this, &CustomSplitter::onSplitterMoved);
}
