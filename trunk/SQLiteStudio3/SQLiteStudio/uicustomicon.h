#ifndef UICUSTOMICON_H
#define UICUSTOMICON_H

#include "uiloaderpropertyhandler.h"

class UiCustomIcon : public UiLoaderPropertyHandler
{
    public:
        UiCustomIcon();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);
};

#endif // UICUSTOMICON_H
