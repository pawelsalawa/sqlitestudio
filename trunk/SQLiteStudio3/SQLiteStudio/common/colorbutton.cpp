#include "colorbutton.h"
#include <QResizeEvent>
#include <QColorDialog>

ColorButton::ColorButton(QWidget *parent) :
    QPushButton(parent)
{
    setFixedWidth(height()*2);
    setColor(Qt::black);
    connect(this, SIGNAL(clicked()), this, SLOT(pickColor()));
}

QColor ColorButton::getColor() const
{
    return color;
}

void ColorButton::setColor(const QColor& value)
{
    color = value;
    QPixmap pix(iconSize());
    pix.fill(color);
    setIcon(pix);
    emit colorChanged(color);
}

void ColorButton::pickColor()
{
    QColor newColor = QColorDialog::getColor(color, parentWidget(), tr("Pick a color"));
    if (!newColor.isValid())
        return;

    setColor(newColor);
}

void ColorButton::resizeEvent(QResizeEvent* e)
{
    setFixedWidth(e->size().height()*2);
}
