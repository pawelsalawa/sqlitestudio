#include "style.h"
#include "themetuner.h"
#include "common/global.h"
#include <QApplication>
#include <QToolTip>

Style* Style::instance = nullptr;

Style* Style::getInstance()
{
    if (instance == nullptr)
        instance = new Style(QApplication::style());

    return instance;
}

void Style::cleanUp()
{
    if (instance)
        safe_delete(instance);
}

const ExtendedPalette& Style::extendedPalette() const
{
    return extPalette;
}

void Style::setStyle(QStyle *style, const QString &styleName)
{
    setBaseStyle(style);

    QApplication::setStyle(style);
    QApplication::setPalette(style->standardPalette());
    THEME_TUNER->tuneTheme(styleName);
    QToolTip::setPalette(style->standardPalette());

    extPalette.styleChanged(style, styleName);
}

Style::Style(QStyle *style)
    : QProxyStyle(style)
{
}
