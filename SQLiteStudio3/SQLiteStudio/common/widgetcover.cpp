#include "widgetcover.h"
#include <QVariantAnimation>
#include <QDebug>
#include <QGraphicsBlurEffect>
#include <QPushButton>
#include <QGridLayout>

WidgetCover::WidgetCover(QWidget *parent) :
    QWidget(parent)
{
    init();
}

WidgetCover::WidgetCover(const QEasingCurve& easingCurve, QWidget* parent)
    : QWidget(parent), easingCurve(easingCurve)
{
    init();
}

WidgetCover::~WidgetCover()
{
    interruptAction();
}

void WidgetCover::init()
{
    setLayout(new QGridLayout(this));
    layout()->setAlignment(Qt::AlignCenter);

    container = new QWidget(this);
    container->setVisible(false);
    layout()->addWidget(container);

    containerLayout = new QGridLayout(container);
    containerLayout->setSizeConstraint(QLayout::SetFixedSize);

    animation = new QVariantAnimation(this);
    animation->setEasingCurve(easingCurve);
    animation->setDuration(duration);
    connect(animation, SIGNAL(valueChanged(QVariant)), this, SLOT(animationUpdate(QVariant)));
    connect(animation, SIGNAL(finished()), this, SLOT(animationFinished()));

    setAutoFillBackground(true);
    resetBackground();
    move(0, 0);
    widgetResized();
}

void WidgetCover::interruptAction()
{
    setVisible(false);
    animation->stop();
}

void WidgetCover::resetBackground()
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QColor(0, 0, 0, 0));
    setPalette(pal);
}

void WidgetCover::animationUpdate(const QVariant& value)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, value.value<QColor>());
    setPalette(pal);
}

void WidgetCover::animationFinished()
{
    switch (actionInProgres)
    {
        case Action::HIDING:
        {
            setVisible(false);
            resetBackground();
            break;
        }
        case Action::SHOWING:
        {
            container->setVisible(true);
            break;
        }
        default:
            break;
    }

    actionInProgres = Action::NONE;
}

void WidgetCover::widgetResized()
{
//    qDebug() << parentWidget()->size();
    setFixedSize(parentWidget()->size());
}

void WidgetCover::show()
{
    if (actionInProgres == Action::SHOWING)
        return;

    if (actionInProgres == Action::HIDING)
        animation->stop();

    actionInProgres = Action::SHOWING;

    QPalette pal = palette();
    animation->setStartValue(QVariant(pal.brush(QPalette::Window).color()));
    animation->setEndValue(QVariant(QColor(0, 0, 0, transparency)));
    setVisible(true);
    container->setVisible(true);
    animation->start();
}

void WidgetCover::hide()
{
    if (actionInProgres == Action::HIDING)
        return;

    if (actionInProgres == Action::SHOWING)
        animation->stop();

    actionInProgres = Action::HIDING;

    container->setVisible(false);

    QPalette pal = palette();
    animation->setStartValue(QVariant(pal.brush(QPalette::Window).color()));
    animation->setEndValue(QVariant(QColor(0, 0, 0, 0)));
    animation->start();
}

QEasingCurve WidgetCover::getEasingCurve() const
{
    return easingCurve;
}

void WidgetCover::setEasingCurve(const QEasingCurve& value)
{
    easingCurve = value;
    animation->setEasingCurve(easingCurve);
}

int WidgetCover::getDuration() const
{
    return duration;
}

void WidgetCover::setDuration(int value)
{
    duration = value;
    animation->setDuration(duration);
}

int WidgetCover::getTransparency() const
{
    return transparency;
}

void WidgetCover::setTransparency(int value)
{
    if (value < 0)
        value = 0;

    if (value > 255)
        value = 255;

    transparency = value;
}

QGridLayout* WidgetCover::getContainerLayout()
{
    return containerLayout;
}
