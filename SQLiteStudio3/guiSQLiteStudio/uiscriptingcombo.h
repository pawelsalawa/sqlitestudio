#ifndef UISCRIPTINGCOMBO_H
#define UISCRIPTINGCOMBO_H

#include "guiSQLiteStudio_global.h"
#include "uiloaderpropertyhandler.h"

class GUI_API_EXPORT UiScriptingCombo : public UiLoaderPropertyHandler
{
    public:
        UiScriptingCombo();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);
};

#endif // UISCRIPTINGCOMBO_H
