#include "widgetstateindicator.h"
#include "iconmanager.h"
#include "common/unused.h"
#include "uiutils.h"
#include "mdichild.h"
#include <QLabel>
#include <QCheckBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QDebug>
#include <QScrollArea>

QHash<QWidget*,WidgetStateIndicator*> WidgetStateIndicator::instances;

WidgetStateIndicator::WidgetStateIndicator(QWidget *widget) :
    QObject(widget), widget(widget)
{
    widget->installEventFilter(this);
    detectWindowParent();
    initEffects();
    initLabel();
    updateMode();
    updatePosition();
    finalInit();
}

WidgetStateIndicator::~WidgetStateIndicator()
{
    instances.remove(widget);
    widget->removeEventFilter(this);
    windowParent->removeEventFilter(this);
}

void WidgetStateIndicator::initLabel()
{
    label = new QLabel();
    label->setMargin(0);
    label->installEventFilter(this);
    label->setGraphicsEffect(highlightingEffect);

    labelParent = new QWidget(windowParent);
    labelParent->setLayout(new QHBoxLayout());
    labelParent->layout()->setMargin(0);
    labelParent->layout()->addWidget(label);
    labelParent->setGraphicsEffect(glowEffect);
}

void WidgetStateIndicator::initEffects()
{
    initGlowEffects();
    initHighlightingEffects();
    initAnimations();
}

void WidgetStateIndicator::initGlowEffects()
{
    glowEffect = new QGraphicsDropShadowEffect();
    glowEffect->setBlurRadius(10.0);
    glowEffect->setOffset(0.0);
    glowEffect->setEnabled(true);
}

void WidgetStateIndicator::initHighlightingEffects()
{
    highlightingEffect = new QGraphicsColorizeEffect();
    highlightingEffect->setColor(Qt::white);
    highlightingEffect->setStrength(0.3);
    highlightingEffect->setEnabled(false);
}

void WidgetStateIndicator::initAnimations()
{
    animation = new QSequentialAnimationGroup(this);
    animation->setLoopCount(-1);

    // Animation of glow efect
    QPropertyAnimation* varAnim = new QPropertyAnimation(glowEffect, "blurRadius");
    varAnim->setStartValue(3.0);
    varAnim->setEndValue(14.0);
    varAnim->setEasingCurve(QEasingCurve::InOutCubic);
    varAnim->setDuration(300);
    animation->addAnimation(varAnim);

    varAnim = new QPropertyAnimation(glowEffect, "blurRadius");
    varAnim->setStartValue(14.0);
    varAnim->setEndValue(3.0);
    varAnim->setEasingCurve(QEasingCurve::InOutCubic);
    varAnim->setDuration(300);
    animation->addAnimation(varAnim);
}

void WidgetStateIndicator::finalInit()
{
    label->setFixedSize(label->pixmap()->size());
    labelParent->setFixedSize(label->pixmap()->size());
    widgetVisible = widget->isVisible();
    labelParent->setVisible(false);
}

void WidgetStateIndicator::setMessage(const QString& msg)
{
    message = msg;
    label->setToolTip(message);
    if (!msg.isNull())
        label->setCursor(Qt::WhatsThisCursor);
    else
        label->unsetCursor();
}

void WidgetStateIndicator::clearMessage()
{
    message = QString::null;
    label->setToolTip(message);
    label->unsetCursor();
}

void WidgetStateIndicator::detectWindowParent()
{
    if (windowParent)
        windowParent->removeEventFilter(this);

    windowParent = findParentWindow(widget);
    windowParent->installEventFilter(this);
    if (labelParent)
        labelParent->setParent(windowParent);
}

void WidgetStateIndicator::setMode(WidgetStateIndicator::Mode mode)
{
    this->mode = mode;
    updateMode();
}

void WidgetStateIndicator::show(const QString& msg, bool animated)
{
    visibilityRequested = true;
    setMessage(msg);
    if (animated && animation->state() != QAbstractAnimation::Running)
        animation->start();

    updateVisibility();
}

void WidgetStateIndicator::hide()
{
    visibilityRequested = false;
    clearMessage();
    if (animation->state() == QAbstractAnimation::Running)
        animation->stop();

    updateVisibility();
}

void WidgetStateIndicator::setVisible(bool visible, const QString& msg)
{
//    if (visible == labelParent->isVisible() && msg == message)
//        return;

    if (visible)
        show(msg);
    else
        hide();
}

void WidgetStateIndicator::release()
{
    setVisible(false);
    instances.remove(widget);
    deleteLater();
}

void WidgetStateIndicator::info(const QString& msg, bool animated)
{
    setMode(Mode::INFO);
    show(msg, animated);
}

void WidgetStateIndicator::warn(const QString& msg, bool animated)
{
    setMode(Mode::WARNING);
    show(msg, animated);
}

void WidgetStateIndicator::error(const QString& msg, bool animated)
{
    setMode(Mode::ERROR);
    show(msg, animated);
}

void WidgetStateIndicator::hint(const QString& msg, bool animated)
{
    setMode(Mode::HINT);
    show(msg, animated);
}

bool WidgetStateIndicator::exists(QWidget* widget)
{
    return instances.contains(widget);
}

WidgetStateIndicator* WidgetStateIndicator::getInstance(QWidget* widget)
{
    if (!instances.contains(widget))
        instances[widget] = new WidgetStateIndicator(widget);

    return instances[widget];
}

bool WidgetStateIndicator::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == widget)
    {
        switch (ev->type())
        {
            case QEvent::Move:
            case QEvent::Resize:
            case QEvent::Scroll:
                updatePosition();
                break;
            case QEvent::Show:
                widgetVisible = true;
                updateVisibility();
                break;
            case QEvent::Hide:
                widgetVisible = false;
                updateVisibility();
                break;
            case QEvent::EnabledChange:
                updateVisibility();
                break;
            default:
                break;
        }
    }
    else if (obj == windowParent)
    {
        switch (ev->type())
        {
            case QEvent::ParentChange:
                detectWindowParent();
                break;
            default:
                break;
        }
    }
    else if (obj == label)
    {
        if (ev->type() == QEvent::Enter)
            highlightingEffect->setEnabled(true);
        else if (ev->type() == QEvent::Leave)
            highlightingEffect->setEnabled(false);
    }

    return false;
}

void WidgetStateIndicator::updateMode()
{
    switch (mode)
    {
        case Mode::ERROR:
            label->setPixmap(ICONS.INDICATOR_ERROR);
            glowEffect->setColor(Qt::red);
            break;
        case Mode::WARNING:
            label->setPixmap(ICONS.INDICATOR_WARN);
            glowEffect->setColor(Qt::darkYellow);
            break;
        case Mode::INFO:
            label->setPixmap(ICONS.INDICATOR_INFO);
            glowEffect->setColor(Qt::blue);
            break;
        case Mode::HINT:
            label->setPixmap(ICONS.INDICATOR_HINT);
            glowEffect->setColor(Qt::darkCyan);
            break;
    }
}

void WidgetStateIndicator::updatePosition()
{
    QPoint xy = widget->mapTo(windowParent, QPoint(0,0));
    QPoint diff;
    if (dynamic_cast<QLabel*>(widget))
        diff = QPoint(-6, -2);
    else if (dynamic_cast<QCheckBox*>(widget))
        diff = QPoint(-6, -2);
    else
        diff = QPoint(-4, -4);

    labelParent->move(xy + diff);
}

void WidgetStateIndicator::updateVisibility()
{
    if (shouldHide())
    {
        labelParent->setVisible(false);
    }
    else if (shouldShow())
    {
        updatePosition();
        labelParent->setVisible(true);
    }
}

bool WidgetStateIndicator::shouldHide()
{
    if (!labelParent->isVisible())
        return false;

    if (!widgetVisible)
        return true;

    if (!visibilityRequested)
        return true;

    if (!widget->isEnabled())
        return true;

    return false;
}

bool WidgetStateIndicator::shouldShow()
{
    if (labelParent->isVisible())
        return false;

    if (!widget->isEnabled())
        return false;

    if (!widgetVisible)
        return false;

    if (!visibilityRequested)
        return false;

    return true;
}

QWidget* WidgetStateIndicator::findParentWindow(QWidget* w)
{
    while (w && !w->windowFlags().testFlag(Qt::Window) && !dynamic_cast<QScrollArea*>(w) && !dynamic_cast<MdiChild*>(w))
        w = w->parentWidget();

    if (dynamic_cast<QScrollArea*>(w))
        return dynamic_cast<QScrollArea*>(w)->widget();

    return w;
}
