#ifndef THEMETUNER_H
#define THEMETUNER_H

#include <QObject>
#include <QString>
#include <QHash>

class ThemeTuner : public QObject
{
        Q_OBJECT

    public:
        void tuneTheme(const QString& themeName);
        void tuneCurrentTheme();
        void manageCompactLayout(QWidget* w);
        void manageCompactLayout(QList<QWidget*> wList);
        QString getDefaultCss(const QString& themeName = QString()) const;

        static ThemeTuner* getInstance();
        static void cleanUp();

    private:
        ThemeTuner(QObject* parent = 0);

        void init();
        void tuneCss(const QString& themeName);
        void tuneMacx();
        void applyCss(const QString& css);

        QString defaultGeneralCss;
        QHash<QString, QString> defaultPerStyleCss;
        QList<QWidget*> widgetsForCompactLayout;

        static ThemeTuner* instance;

    private slots:
        void handleWidgetDestroyed();
        void handleCompactLayoutChange(const QVariant& newValue);
};

#define THEME_TUNER ThemeTuner::getInstance()

#endif // THEMETUNER_H
