#include "themetuner.h"
#include "uiconfig.h"
#include "mainwindow.h"
#include "uiconfig.h"
#include "style.h"
#include <QApplication>
#include <QFile>
#include <QStyle>
#include <QDebug>
#include <QWizard>

ThemeTuner* ThemeTuner::instance = nullptr;

ThemeTuner::ThemeTuner(QObject* parent) :
    QObject(parent)
{
    init();
}

void ThemeTuner::tuneTheme(const QString& themeName)
{
    tuneCss(themeName);
}

void ThemeTuner::tuneCurrentTheme()
{
    tuneTheme(STYLE->name());
}

void ThemeTuner::manageCompactLayout(QWidget* w)
{
    manageCompactLayout(QList<QWidget*>({w}));
}

void ThemeTuner::manageCompactLayout(QList<QWidget*> wList)
{
    widgetsForCompactLayout += wList;
    for (QWidget*& w : wList)
        connect(w, SIGNAL(destroyed()), this, SLOT(handleWidgetDestroyed()));

    handleCompactLayoutChange(CFG_UI.General.CompactLayout.get());
}

QString ThemeTuner::getDefaultCss(const QString& themeName) const
{
    QString css = defaultGeneralCss;
    QString lowerTheme = themeName.toLower();
    if (!themeName.isNull() && defaultPerStyleCss.contains(lowerTheme))
        css += "\n" + defaultPerStyleCss[lowerTheme];

    return css;
}

ThemeTuner* ThemeTuner::getInstance()
{
    if (!instance)
        instance = new ThemeTuner();

    return instance;
}

void ThemeTuner::cleanUp()
{
    if (instance)
        safe_delete(instance);
}

void ThemeTuner::init()
{
    QFile f(":/css/general.css");
    if (!f.open(QIODevice::ReadOnly))
    {
        qCritical() << "Could not open general.css";
        return;
    }

    defaultGeneralCss = QString::fromLatin1(f.readAll());
    f.close();

    connect(CFG_UI.General.CompactLayout, SIGNAL(changed(QVariant)), this, SLOT(handleCompactLayoutChange(QVariant)));
}

void ThemeTuner::tuneCss(const QString& themeName)
{
    if (!CFG_UI.General.CustomCss.get().isNull())
    {
        applyCss(CFG_UI.General.CustomCss.get());
        return;
    }

    applyCss(getDefaultCss(themeName));
}

void ThemeTuner::applyCss(const QString& css)
{
    MAINWINDOW->setStyleSheet(css);
}

void ThemeTuner::handleWidgetDestroyed()
{
    QWidget* w = dynamic_cast<QWidget*>(sender());
    if (!w)
        return;

    widgetsForCompactLayout.removeOne(w);
}

void ThemeTuner::handleCompactLayoutChange(const QVariant& newValue)
{
    if (newValue.toBool())
    {
        for (QWidget*& w : widgetsForCompactLayout)
        {
            w->layout()->setContentsMargins(0, 0, 0, 0);
            w->layout()->setSpacing(0);
        }
    }
    else
    {
        for (QWidget*& w : widgetsForCompactLayout)
        {
            w->layout()->setContentsMargins(-1, -1, -1, -1);
            w->layout()->setSpacing(-1);
        }
    }
}

