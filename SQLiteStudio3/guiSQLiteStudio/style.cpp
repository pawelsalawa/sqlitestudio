#include "style.h"
#include "themetuner.h"
#include "mainwindow.h"
#include <QApplication>
#include <QToolTip>
#include <QDebug>

Style* Style::instance = nullptr;

Style* Style::getInstance()
{
    if (instance == nullptr)
        instance = new Style(QApplication::style());

    return instance;
}


bool Style::isDark(const QStyle* style)
{
    return style->standardPalette().text().color().value() >= 80;
}

const ExtendedPalette& Style::extendedPalette() const
{
    return extPalette;
}

void Style::setStyle(QStyle *style, const QString &styleName)
{
    setBaseStyle(style);

    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

    QApplication::setPalette(initialPalette); // reset palette, cause styles don't provide
                                              // full palette when changed in runtime (i.e. windowsvista)
    QApplication::setStyle(this);
    QApplication::setPalette(standardPalette());
    THEME_TUNER->tuneTheme(styleName);
    QToolTip::setPalette(standardPalette());
    extPalette.styleChanged(this, styleName);
    MAINWINDOW->getMdiArea()->setBackground(extPalette.mdiAreaBase());
}

QString Style::name() const
{
    return baseStyle()->objectName();
}

bool Style::isDark() const
{
    return isDark(this);
}

Style::Style(QStyle *style)
    : QProxyStyle(style)
{
    initialPalette = style->standardPalette();
    extPalette.styleChanged(this, name());
}
