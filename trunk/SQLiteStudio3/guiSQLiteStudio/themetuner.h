#ifndef THEMETUNER_H
#define THEMETUNER_H

#include <QString>
#include <QHash>

class ThemeTuner
{
    public:
        static void tuneTheme(const QString& themeName);
        static void tuneCurrentTheme();
        static void staticInit();

    private:
        static void tuneCss(const QString& themeName);
        static void tuneMacx();
        static void applyCss(const QString& css);

        static QString defaultGeneralCss;
        static QHash<QString, QString> defaultPerStyleCss;
};

#endif // THEMETUNER_H
