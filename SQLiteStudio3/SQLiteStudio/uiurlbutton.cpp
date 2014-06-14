#include "uiurlbutton.h"
#include <QAbstractButton>
#include <QDesktopServices>
#include <QUrl>

UiUrlButton::UiUrlButton()
{
}


const char* UiUrlButton::getPropertyName() const
{
    return "openUrl";
}

void UiUrlButton::handle(QWidget* widget, const QVariant& value)
{
    QAbstractButton* btn = dynamic_cast<QAbstractButton*>(widget);
    QString url = value.toString();
    if (btn)
    {
        QObject::connect(btn, &QAbstractButton::clicked, [url](bool)
        {
            QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
        });
    }
}
