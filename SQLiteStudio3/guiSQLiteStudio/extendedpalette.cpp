#include "extendedpalette.h"
#include "common/unused.h"
#include <QStyle>
#include <QDebug>

ExtendedPalette::ExtendedPalette()
{
}

QBrush ExtendedPalette::editorString() const
{
    return editorStringBrush;
}

void ExtendedPalette::setEditorString(const QBrush &value)
{
    editorStringBrush = value;
}

QBrush ExtendedPalette::editorLineBase() const
{
    return editorLineBaseBrush;
}

void ExtendedPalette::setEditorLineBase(const QBrush &value)
{
    editorLineBaseBrush = value;
}

bool ExtendedPalette::styleChanged(QStyle *style, const QString &themeName)
{
    UNUSED(themeName);
    QPalette stdPalette = style->standardPalette();
    if (stdPalette == lastPalette)
        return false;

    lastPalette = stdPalette;
    bool isDark = stdPalette.base().color().lightness() < 128;

    static const QColor stdStrColor = QColor(Qt::green);
    static const QColor stdExprColor = QColor(Qt::magenta);
    if (stdPalette.text().color().lightness() >= 128)
        editorStringBrush = stdStrColor.lighter();
    else
        editorStringBrush = stdStrColor.darker();

    if (stdPalette.text().color().lightness() >= 128)
        editorExpressionBrush = stdExprColor.lighter();
    else
        editorExpressionBrush = stdExprColor;

    if (isDark)
    {
        mdiAreaBaseBrush = stdPalette.alternateBase();
        editorLineNumberBaseBrush = stdPalette.base().color().lighter(130);
        editorLineBaseBrush = stdPalette.base().color().lighter(130);
        editorCurrentQueryBrush = stdPalette.base().color().lighter(120);
    }
    else
    {
        editorLineNumberBaseBrush = stdPalette.base().color().darker(120);
        editorLineBaseBrush = stdPalette.base().color().darker(120);
        editorCurrentQueryBrush = stdPalette.base().color().darker(110);
        mdiAreaBaseBrush = QColor(138, 138, 138);
    }

    return true;
}

QBrush ExtendedPalette::mdiAreaBase() const
{
    return mdiAreaBaseBrush;
}

void ExtendedPalette::setMdiAreaBase(const QBrush& value)
{
    mdiAreaBaseBrush = value;
}

QBrush ExtendedPalette::editorExpression() const
{
    return editorExpressionBrush;
}

void ExtendedPalette::setEditorExpression(const QBrush& value)
{
    editorExpressionBrush = value;
}

const QBrush& ExtendedPalette::editorCurrentQueryBase() const
{
    return editorCurrentQueryBrush;
}

void ExtendedPalette::setEditorCurrentQueryBase(const QBrush& value)
{
    editorCurrentQueryBrush = value;
}

const QBrush& ExtendedPalette::editorLineNumberBase() const
{
    return editorLineNumberBaseBrush;
}

void ExtendedPalette::setEditorLineNumberBaseBrush(const QBrush& newEditorLineNumberBaseBrush)
{
    editorLineNumberBaseBrush = newEditorLineNumberBaseBrush;
}
