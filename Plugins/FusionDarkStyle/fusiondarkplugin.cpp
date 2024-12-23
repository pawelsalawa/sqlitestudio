#include "fusiondarkplugin.h"
#include "themetuner.h"
#include <QProxyStyle>
#include <QPalette>
#include <QDebug>
#include <QWizard>

class FusionDarkStyle : public QProxyStyle
{
    public:
        FusionDarkStyle();

        QPalette standardPalette() const;

    private:
        QPalette darkPalette;
};

FusionDarkStyle::FusionDarkStyle()
    : QProxyStyle("fusion")
{
    setObjectName(FusionDarkPlugin::STYLE_NAME);

    QColor lightGray(190, 190, 190);
    QColor gray(128, 128, 128);
    QColor midDarkGray(100, 100, 100);
    QColor darkGray(53, 53, 53);
    QColor black(25, 25, 25);
    QColor blue(42, 130, 218);

    darkPalette.setColor(QPalette::Window, darkGray);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, black);
    darkPalette.setColor(QPalette::AlternateBase, darkGray);
    darkPalette.setColor(QPalette::ToolTipBase, darkGray);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, lightGray);
    darkPalette.setColor(QPalette::Button, darkGray);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Link, blue);
    darkPalette.setColor(QPalette::Highlight, blue);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Light, blue);
    darkPalette.setColor(QPalette::Dark, midDarkGray);

    darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
}

QPalette FusionDarkStyle::standardPalette() const
{
    return darkPalette;
}

FusionDarkPlugin::FusionDarkPlugin(QObject *parent)
    : QStylePlugin(parent)
{
}

FusionDarkPlugin::~FusionDarkPlugin()
{
}

QStyle *FusionDarkPlugin::create(const QString &key)
{
    if (key.toLower() == STYLE_NAME)
        return new FusionDarkStyle();

    return nullptr;
}
