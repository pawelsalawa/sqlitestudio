#ifndef UIURLBUTTON_H
#define UIURLBUTTON_H

#include "guiSQLiteStudio_global.h"
#include "uiloaderpropertyhandler.h"

class GUI_API_EXPORT UiUrlButton : public UiLoaderPropertyHandler
{
    public:
        UiUrlButton();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);
};

#endif // UIURLBUTTON_H
