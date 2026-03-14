#include "materialswitch.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

static constexpr int SWITCH_W = 40;
static constexpr int SWITCH_H = 30;

MaterialSwitch::MaterialSwitch(QWidget *parent)
    : QCheckBox(parent),
      offsetValue(0.0)
{
    setCursor(Qt::PointingHandCursor);

    animation = new QPropertyAnimation(this, "offset", this);
    animation->setDuration(120);
    animation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(this, &QCheckBox::toggled, this, &MaterialSwitch::startTransition);
}

MaterialSwitch::MaterialSwitch(const QString& title, QWidget* parent) :
    MaterialSwitch(parent)
{
    setText(title);
}

QSize MaterialSwitch::sizeHint() const
{
    QFontMetrics fm(font());

    int textW = fm.horizontalAdvance(text());
    int textH = fm.height();

    int w = SWITCH_W;
    int h = SWITCH_H;

    if (!text().isEmpty())
    {
        w += m_spacing + textW;
        h = std::max(h, textH);
    }

    return QSize(w, h);
}

qreal MaterialSwitch::offset() const
{
    return offsetValue;
}

void MaterialSwitch::setOffset(qreal value)
{
    offsetValue = value;
    update();
}

void MaterialSwitch::setLabelPosition(LabelPosition pos)
{
    labelPos = pos;
    updateGeometry();
    update();
}

void MaterialSwitch::setLabelAlignment(Qt::Alignment align)
{
    labelAlign = align;
    update();
}

Qt::Alignment MaterialSwitch::labelAlignment() const
{
    return labelAlign;
}

MaterialSwitch::LabelPosition MaterialSwitch::labelPosition() const
{
    return labelPos;
}

void MaterialSwitch::resizeEvent(QResizeEvent* e)
{
    QCheckBox::resizeEvent(e);
    offsetValue = isChecked() ? 1.0 : 0.0;
}

bool MaterialSwitch::hitButton(const QPoint& pos) const
{
    return rect().contains(pos);
}

void MaterialSwitch::startTransition(bool checked)
{
    animation->stop();
    animation->setStartValue(offsetValue);
    animation->setEndValue(checked ? 1.0 : 0.0);
    animation->start();
}

QRect MaterialSwitch::switchRect() const
{
    int y = (height() - SWITCH_H) / 2;

    if (text().isEmpty())
        return QRect(0, y, SWITCH_W, SWITCH_H);

    if (labelPos == LabelLeft)
        return QRect(width() - SWITCH_W, y, SWITCH_W, SWITCH_H);
    else
        return QRect(0, y, SWITCH_W, SWITCH_H);
}

QRect MaterialSwitch::labelRect() const
{
    if (text().isEmpty())
        return QRect();

    if (labelPos == LabelLeft)
        return QRect(0, 0, width() - SWITCH_W - m_spacing, height());
    else
        return QRect(SWITCH_W + m_spacing, 0, width() - SWITCH_W - m_spacing, height());
}

void MaterialSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRect sRect = switchRect();
    const QPalette pal = palette();

    QColor window = pal.color(QPalette::Window);
    QColor highlight = pal.color(QPalette::Highlight);

    bool darkTheme = window.lightness() < 128;

    QColor trackOff = darkTheme ? window.lighter(140)
                                : window.darker(110);

    QColor trackOn = highlight;

    QColor thumbColor = pal.color(QPalette::Base);

    if (!isEnabled())
    {
        trackOff = pal.color(QPalette::Disabled, QPalette::Mid);
        trackOn = pal.color(QPalette::Disabled, QPalette::Highlight);
        thumbColor = pal.color(QPalette::Disabled, QPalette::Base);
    }

    constexpr int margin = 2;
    constexpr qreal trackHeight = 12;
    constexpr qreal thumbRadius = 8;

    QRectF trackRect(
        sRect.x() + margin,
        sRect.center().y() - trackHeight/2,
        sRect.width() - 2*margin,
        trackHeight
    );

    QColor trackColor = isChecked() ? trackOn : trackOff;

    // TRACK
    p.setPen(Qt::NoPen);
    p.setBrush(trackColor);
    p.drawRoundedRect(trackRect, trackHeight/2, trackHeight/2);

    // INNER SHADOW
    QPainterPath trackPath;
    QRectF shadowRect = trackRect;
    trackPath.addRoundedRect(shadowRect, trackHeight/2, trackHeight/2);

    p.save();
    p.setClipPath(trackPath);

    int shadowTopWidth = 6;
    QColor shadowColor = darkTheme ? QColor(0,0,0,150) : QColor(0,0,0,80);
    QColor shadowMiddleColor = darkTheme ? QColor(0,0,0,80) : QColor(0,0,0,40);

    QLinearGradient gTop(shadowRect.topLeft(), shadowRect.topLeft() + QPointF(0, shadowTopWidth));
    gTop.setColorAt(0, shadowColor);
    gTop.setColorAt(0.4, shadowMiddleColor);
    gTop.setColorAt(1, Qt::transparent);
    p.fillRect(QRectF(shadowRect.left(),
                      shadowRect.top(),
                      shadowRect.width(),
                      shadowTopWidth),
               gTop);

    p.restore();

    // THUMB POSITION
    qreal travel = sRect.width() - 2*margin - 2*thumbRadius;
    qreal x = sRect.x() + margin + offsetValue * travel;

    QPointF center(x + thumbRadius, sRect.center().y() - 0.5);

    // THUMB SHADOW
    QColor thumbShadow = darkTheme
                         ? QColor(0,0,0,160)
                         : QColor(0,0,0,90);

    p.setBrush(thumbShadow);
    p.setPen(Qt::NoPen);
    for (int i = 3; i >= 1; --i)
    {
        QColor c(0,0,0,25*i);
        p.setBrush(c);
        p.drawEllipse(center + QPointF(0,1), thumbRadius+(0.5*i), thumbRadius+(0.5*i));
    }

    // THUMB
    p.setBrush(thumbColor);
    p.drawEllipse(center, thumbRadius, thumbRadius);

    // LABEL
    if (!text().isEmpty())
    {
        QRect tRect = labelRect().translated(0, -2);

        p.setPen(isEnabled()
               ? pal.color(QPalette::WindowText)
               : pal.color(QPalette::Disabled, QPalette::WindowText));

        p.drawText(tRect, labelAlign | Qt::AlignVCenter, text());
    }
}
