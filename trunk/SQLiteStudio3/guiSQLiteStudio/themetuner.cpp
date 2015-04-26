#include "themetuner.h"
#include "uiconfig.h"
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QStyle>
#include <QDebug>

QString ThemeTuner::defaultGeneralCss;
QHash<QString, QString> ThemeTuner::defaultPerStyleCss;

void ThemeTuner::tuneTheme(const QString& themeName)
{
    tuneCss(themeName);
    if (themeName == "macintosh")
        tuneMacx();
}

void ThemeTuner::tuneCurrentTheme()
{
    tuneTheme(QApplication::style()->objectName());
}

void ThemeTuner::staticInit()
{
    QFile f(":/css/general.css");
    if (!f.open(QIODevice::ReadOnly))
    {
        qCritical() << "Could not open general.css";
        return;
    }

    defaultGeneralCss = QString::fromLatin1(f.readAll());
    f.close();
}

void ThemeTuner::tuneCss(const QString& themeName)
{
    if (!CFG_UI.General.CustomCss.get().isNull())
    {
        applyCss(CFG_UI.General.CustomCss.get());
        return;
    }

    applyCss(defaultGeneralCss);
    QString lowerTheme = themeName.toLower();
    if (defaultPerStyleCss.contains(lowerTheme))
        applyCss(defaultPerStyleCss[lowerTheme]);
}

void ThemeTuner::tuneMacx()
{
    // TODO
}

void ThemeTuner::applyCss(const QString& css)
{
    MAINWINDOW->setStyleSheet(css);
}

