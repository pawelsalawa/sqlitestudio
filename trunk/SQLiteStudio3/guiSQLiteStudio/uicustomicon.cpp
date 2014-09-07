#include "uicustomicon.h"
#include "iconmanager.h"
#include <QLabel>
#include <QAbstractButton>

#define TRY_ICON_WITH(Type, Widget, Method, Icon) \
    if (dynamic_cast<Type*>(Widget))\
    {\
        dynamic_cast<Type*>(Widget)->Method(Icon);\
        return;\
    }

UiCustomIcon::UiCustomIcon()
{
}

const char* UiCustomIcon::getPropertyName() const
{
    return "customIcon";
}

void UiCustomIcon::handle(QWidget* widget, const QVariant& value)
{
    QString iconName = value.toString();
    QIcon* icon = ICONMANAGER->getIcon(iconName);
    if (!icon)
        return;

    TRY_ICON_WITH(QAbstractButton, widget, setIcon, *icon);
}
