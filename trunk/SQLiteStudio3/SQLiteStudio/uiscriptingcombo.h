#ifndef UISCRIPTINGCOMBO_H
#define UISCRIPTINGCOMBO_H

#include "uiloaderpropertyhandler.h"

class UiScriptingCombo : public UiLoaderPropertyHandler
{
    public:
        UiScriptingCombo();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);
};

#endif // UISCRIPTINGCOMBO_H
