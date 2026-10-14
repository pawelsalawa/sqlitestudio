#include "extendedpalette.h"
#include <QStyle>
#include <QDebug>

ExtendedPalette::ExtendedPalette()
{
}

bool ExtendedPalette::styleChanged(QStyle *style, const QString &themeName)
{
    Q_UNUSED(themeName);
    QPalette stdPalette = style->standardPalette();
    // A default QPalette can already match the style before any brushes are initialized.
    if (initialized && stdPalette == initializedForPalette)
        return false;

    initializedForPalette = stdPalette;
    initialized = true;
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
        mdiAreaBaseBrush = stdPalette.base().color().lighter(120);
        editorLineNumberBaseBrush = stdPalette.base().color().lighter(130);
        editorLineBaseBrush = stdPalette.base().color().lighter(130);
        editorCurrentQueryBrush = stdPalette.base().color().lighter(120);
        editorCommentBrush = stdPalette.text().color().darker(210);
    }
    else
    {
        editorLineNumberBaseBrush = stdPalette.base().color().darker(120);
        editorLineBaseBrush = stdPalette.base().color().darker(120);
        editorCurrentQueryBrush = stdPalette.base().color().darker(110);
        editorCommentBrush = stdPalette.base().color().darker(160);
        mdiAreaBaseBrush = stdPalette.base().color().darker(170);
    }

    return true;
}

QBrush ExtendedPalette::editorString() const
{
    return editorStringBrush;
}

QBrush ExtendedPalette::editorLineBase() const
{
    return editorLineBaseBrush;
}

QBrush ExtendedPalette::mdiAreaBase() const
{
    return mdiAreaBaseBrush;
}

QBrush ExtendedPalette::editorExpression() const
{
    return editorExpressionBrush;
}

const QBrush& ExtendedPalette::editorCurrentQueryBase() const
{
    return editorCurrentQueryBrush;
}

const QBrush& ExtendedPalette::editorLineNumberBase() const
{
    return editorLineNumberBaseBrush;
}

QBrush ExtendedPalette::editorComment() const
{
    return editorCommentBrush;
}
