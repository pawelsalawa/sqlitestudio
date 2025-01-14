#include "widgetstateindicator.h"
#include "iconmanager.h"
#include "common/unused.h"
#include "uiutils.h"
#include "mdichild.h"
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QScrollArea>
#include <QToolTip>
#include <QEnterEvent>

QHash<QWidget*,WidgetStateIndicator*> WidgetStateIndicator::instances;

WidgetStateIndicator::WidgetStateIndicator(QWidget *widget) :
    QObject(widget), widget(widget)
{
    widget->installEventFilter(this);
    detectWindowParent();
    initPositionMode();
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
    labelParent->layout()->setContentsMargins(0, 0, 0, 0);
    labelParent->layout()->addWidget(label);
    labelParent->setGraphicsEffect(glowEffect);
}

void WidgetStateIndicator::initEffects()
{
    initGlowEffects();
    initHighlightingEffects();
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

void WidgetStateIndicator::initPositionMode()
{
    if (dynamic_cast<QGroupBox*>(widget))
        positionMode = PositionMode::GROUP_BOX;
    else if (dynamic_cast<QLabel*>(widget))
        positionMode = PositionMode::LABEL;
    else if (dynamic_cast<QCheckBox*>(widget))
        positionMode = PositionMode::CHECK_BOX;
}

void WidgetStateIndicator::finalInit()
{
    QPixmap pixmap = label->pixmap(Qt::ReturnByValue);
    label->setFixedSize(pixmap.size());
    labelParent->setFixedSize(pixmap.size());
    widgetVisible = widget->isVisible();
    labelParent->setVisible(false);
}

void WidgetStateIndicator::setMessage(const QString& msg)
{
    static const QString paraTpl = QStringLiteral("<p>%1</p>");
    if (msg.startsWith("<p>") && msg.endsWith("</p>"))
        message = msg;
    else
        message = paraTpl.arg(msg);

    if (!msg.isNull())
        label->setCursor(Qt::WhatsThisCursor);
    else
        label->unsetCursor();
}

void WidgetStateIndicator::clearMessage()
{
    message = QString();
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

void WidgetStateIndicator::show(const QString& msg)
{
    visibilityRequested = true;
    setMessage(msg);
    updateVisibility();
}

void WidgetStateIndicator::hide()
{
    visibilityRequested = false;
    clearMessage();
    updateVisibility();
}

void WidgetStateIndicator::setVisible(bool visible, const QString& msg)
{
    if (visible)
        show(msg);
    else
        hide();
}

bool WidgetStateIndicator::isVisible() const
{
    return labelParent->isVisible();
}

void WidgetStateIndicator::release()
{
    setVisible(false);
    instances.remove(widget);
    deleteLater();
}

void WidgetStateIndicator::info(const QString& msg)
{
    setMode(Mode::INFO);
    show(msg);
}

void WidgetStateIndicator::warn(const QString& msg)
{
    setMode(Mode::WARNING);
    show(msg);
}

void WidgetStateIndicator::error(const QString& msg)
{
    setMode(Mode::ERROR);
    show(msg);
}

void WidgetStateIndicator::hint(const QString& msg)
{
    setMode(Mode::HINT);
    show(msg);
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
        return eventFilterFromWidget(ev);
    else if (obj == windowParent)
        return eventFilterFromParentWidget(ev);
    else if (obj == label)
        return eventFilterFromIndicatorLabel(ev);

    return false;
}

void WidgetStateIndicator::updateMode()
{
    switch (mode)
    {
        case Mode::ERROR:
            label->setPixmap(ICONS.INDICATOR_ERROR.toQPixmap(12));
            glowEffect->setColor(Qt::red);
            break;
        case Mode::WARNING:
            label->setPixmap(ICONS.INDICATOR_WARN.toQPixmap(12));
            glowEffect->setColor(Qt::darkYellow);
            break;
        case Mode::INFO:
            label->setPixmap(ICONS.INDICATOR_INFO.toQPixmap(12));
            glowEffect->setColor(Qt::blue);
            break;
        case Mode::HINT:
            label->setPixmap(ICONS.INDICATOR_HINT.toQPixmap(12));
            glowEffect->setColor(Qt::darkCyan);
            break;
    }
}

void WidgetStateIndicator::updatePosition()
{
    switch (positionMode)
    {
        case PositionMode::DEFAULT:
            updatePositionDefault();
            break;
        case PositionMode::GROUP_BOX:
            updatePositionGroupBox();
            break;
        case PositionMode::LABEL:
            updatePositionLabel();
            break;
        case PositionMode::CHECK_BOX:
            updatePositionCheckBox();
            break;
    }
}

void WidgetStateIndicator::updatePositionDefault()
{
    QPoint xy = widget->mapTo(windowParent, QPoint(0,0));
    labelParent->move(xy + QPoint(-4, -4));
}

void WidgetStateIndicator::updatePositionGroupBox()
{
    QPoint xy = widget->mapTo(windowParent, QPoint(0,0));

    QGroupBox* gb = dynamic_cast<QGroupBox*>(widget);

    QFont font = gb->font();
    QFontMetrics fm(font);
    QString txt = gb->title();
    QPoint diff(fm.horizontalAdvance(txt), 2);

    labelParent->move(xy + diff);
}

void WidgetStateIndicator::updatePositionLabel()
{
    updatePositionCheckBox(); // currently they're equal
}

void WidgetStateIndicator::updatePositionCheckBox()
{
    QPoint xy = widget->mapTo(windowParent, QPoint(0,0));
    labelParent->move(xy + QPoint(-6, -2));
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

bool WidgetStateIndicator::eventFilterFromWidget(QEvent* ev)
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
    return false;
}

bool WidgetStateIndicator::eventFilterFromParentWidget(QEvent* ev)
{
    switch (ev->type())
    {
        case QEvent::ParentChange:
            detectWindowParent();
            break;
        default:
            break;
    }
    return false;
}

bool WidgetStateIndicator::eventFilterFromIndicatorLabel(QEvent* ev)
{
    switch (ev->type())
    {
        case QEvent::Enter:
        {
            highlightingEffect->setEnabled(true);
            QEnterEvent* e = dynamic_cast<QEnterEvent*>(ev);
            QToolTip::showText(e->globalPosition().toPoint(), message);
            break;
        }
        case QEvent::Leave:
            highlightingEffect->setEnabled(false);
            QToolTip::hideText();
            break;
        default:
            break;
    }
    return false;
}

WidgetStateIndicator::PositionMode WidgetStateIndicator::getPositionMode() const
{
    return positionMode;
}

void WidgetStateIndicator::setPositionMode(const PositionMode& value)
{
    positionMode = value;
}

QWidget* WidgetStateIndicator::findParentWindow(QWidget* w)
{
    while (w && !w->windowFlags().testFlag(Qt::Window) && !dynamic_cast<QScrollArea*>(w) && !dynamic_cast<MdiChild*>(w))
        w = w->parentWidget();

    if (dynamic_cast<QScrollArea*>(w))
        return dynamic_cast<QScrollArea*>(w)->widget();

    return w;
}
