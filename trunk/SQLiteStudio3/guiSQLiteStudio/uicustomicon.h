#ifndef UICUSTOMICON_H
#define UICUSTOMICON_H

#include "guiSQLiteStudio_global.h"
#include "uiloaderpropertyhandler.h"

class GUI_API_EXPORT UiCustomIcon : public UiLoaderPropertyHandler
{
    public:
        UiCustomIcon();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);
};

#endif // UICUSTOMICON_H
