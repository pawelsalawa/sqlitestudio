#ifndef THEMETUNER_H
#define THEMETUNER_H

#include "guiSQLiteStudio_global.h"
#include <functional>
#include <QObject>
#include <QString>
#include <QHash>

class QWizard;

class GUI_API_EXPORT ThemeTuner : public QObject
{
        Q_OBJECT

    public:
        typedef std::function<void(const QString&, QWizard*)> QWizardThemeTuner;

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
        QStringList qwizardThemeTuneRequired;

        static ThemeTuner* instance;

    private slots:
        void handleWidgetDestroyed();
        void handleCompactLayoutChange(const QVariant& newValue);
};

#define THEME_TUNER ThemeTuner::getInstance()



#endif // THEMETUNER_H
