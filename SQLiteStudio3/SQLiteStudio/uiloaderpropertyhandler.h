#ifndef UILOADERPROPERTYHANDLER_H
#define UILOADERPROPERTYHANDLER_H

#include <QVariant>

class QWidget;

class UiLoaderPropertyHandler
{
    public:
        virtual const char* getPropertyName() const = 0;
        virtual void handle(QWidget* widget, const QVariant& value) = 0;
};

#endif // UILOADERPROPERTYHANDLER_H
