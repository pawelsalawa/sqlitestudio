#ifndef UIURLBUTTON_H
#define UIURLBUTTON_H

#include "uiloaderpropertyhandler.h"

class UiUrlButton : public UiLoaderPropertyHandler
{
    public:
        UiUrlButton();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);
};

#endif // UIURLBUTTON_H
