#ifndef FORMMANAGER_H
#define FORMMANAGER_H

#include "mainwindow.h"
#include <QHash>
#include <QString>

class QUiLoader;

class FormManager
{
    public:
        FormManager();
        virtual ~FormManager();

        QWidget* createWidget(const QString& name);
        bool hasWidget(const QString& name);

    private:
        void init();
        void loadRecurently(const QString& path, const QString& prefix = "");
        QString getWidgetName(const QString& path);
        QWidget* createWidgetByFullPath(const QString& path);

        QUiLoader* uiLoader = nullptr;
        QHash<QString,QString> widgetNameToFullPath;
};

#define FORMS MainWindow::getInstance()->getFormManager()

#endif // FORMMANAGER_H
