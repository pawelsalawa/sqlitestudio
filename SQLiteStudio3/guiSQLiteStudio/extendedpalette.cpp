#include "extendedpalette.h"
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

void ExtendedPalette::styleChanged(QStyle *style, const QString &themeName)
{
    QPalette stdPalette = style->standardPalette();
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

    if (themeName.toLower() == "macintosh" && isDark)
        editorLineNumberBaseBrush = stdPalette.alternateBase().color().darker(300);
    else
        editorLineNumberBaseBrush = stdPalette.alternateBase();

    if (isDark)
    {
        editorLineBaseBrush = stdPalette.alternateBase();
        editorCurrentQueryBrush = stdPalette.base().color().lighter(140);
        mdiAreaBaseBrush = stdPalette.alternateBase();
    }
    else
    {
        editorLineBaseBrush = stdPalette.alternateBase().color().darker(103);
        editorCurrentQueryBrush = stdPalette.base().color().darker(103);
        mdiAreaBaseBrush = QColor(138, 138, 138);
    }
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
