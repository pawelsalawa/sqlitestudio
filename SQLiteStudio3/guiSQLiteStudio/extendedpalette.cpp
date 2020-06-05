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

    static const QColor stdAltColor = QColor(Qt::green);
    if (stdPalette.text().color().lightness() >= 128)
        editorStringBrush = QBrush(stdAltColor.lighter());
    else
        editorStringBrush = QBrush(stdAltColor.darker());

    if (themeName.toLower() == "macintosh" && stdPalette.base().color().lightness() < 128)
        editorLineBaseBrush = QBrush(stdPalette.alternateBase().color().darker(300));
    else
        editorLineBaseBrush = stdPalette.alternateBase();
}
