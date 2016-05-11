#include "extlineedit.h"
#include "iconmanager.h"
#include <QStyle>
#include <QAction>
#include <QDebug>

ExtLineEdit::ExtLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    init();
}

ExtLineEdit::ExtLineEdit(const QString& text, QWidget *parent)
    : QLineEdit(text, parent)
{
    init();
}

void ExtLineEdit::init()
{
    connect(this, &QLineEdit::textChanged, this, &ExtLineEdit::handleTextChanged);
}

void ExtLineEdit::updateMinSize()
{
    setMinimumSize(expandingMinWidth, 0);
}

int ExtLineEdit::getExpandingMaxWidth() const
{
    return expandingMaxWidth;
}

void ExtLineEdit::setExpandingMaxWidth(int value)
{
    expandingMaxWidth = value;
    setMaximumWidth(value);
}

void ExtLineEdit::setClearButtonEnabled(bool enable)
{
    QLineEdit::setClearButtonEnabled(enable);
    if (enable)
    {
        // This is a hack to get to know when QLineEdit's clear button is pressed.
        // Unfortunately Qt 5.2 API doesn't provide such information,
        // but we can find QAction responsible for it by its object name
        // and handle its triggered() signal.
        // This is not part of an official Qt's API and may be modified in any Qt version.
        // Ugly, but works.
        static const char* qtClearBtnActionName = "_q_qlineeditclearaction";
        QAction *clearAction = findChild<QAction*>(qtClearBtnActionName);
        if (!clearAction)
        {
            qWarning() << "Could not find 'clear action' in QLineEdit, so 'valueErased()' signal won't be emitted from ExtLineEdit.";
            return;
        }
        connect(clearAction, SIGNAL(triggered()), this, SLOT(checkForValueErased()));
    }
}

void ExtLineEdit::checkForValueErased()
{
    if (text().isEmpty())
        return;

    emit valueErased();
}

bool ExtLineEdit::getExpanding() const
{
    return expanding;
}

void ExtLineEdit::setExpanding(bool value)
{
    expanding = value;
    if (!expanding)
        setFixedWidth(-1);
    else
        setFixedWidth(expandingMinWidth);
}

int ExtLineEdit::getExpandingMinWidth() const
{
    return expandingMinWidth;
}

void ExtLineEdit::setExpandingMinWidth(int value)
{
    expandingMinWidth = value;
    updateMinSize();
}

void ExtLineEdit::handleTextChanged()
{
    QString txt = text();
    if (!expanding)
        return;

    // Text width
    int newWidth = fontMetrics().width(txt);

    // Text margins
    QMargins margins = textMargins();
    newWidth += margins.left() + margins.right();

    // Content margins
    QMargins localContentsMargins = contentsMargins();
    newWidth += localContentsMargins.left() + localContentsMargins.right();

    // Frame
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    newWidth += frameWidth * 2;

    // Extra space
    newWidth += expandingExtraSpace;

    if (newWidth < expandingMinWidth)
        newWidth = expandingMinWidth;
    else if (expandingMaxWidth > 0 && newWidth > expandingMaxWidth)
        newWidth = expandingMaxWidth;

    setFixedWidth(newWidth);
}
