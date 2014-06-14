#include "uiloader.h"
#include "common/unused.h"
#include "uiloaderpropertyhandler.h"
#include "uiscriptingcombo.h"
#include "uiscriptingedit.h"
#include "uicustomicon.h"
#include "uiurlbutton.h"
#include <QComboBox>
#include <QDebug>
#include <QMetaProperty>
#include <QXmlSimpleReader>

UiLoader::UiLoader(QObject *parent) :
    QUiLoader(parent)
{
    registerPropertyHandler(new UiScriptingCombo());
    registerPropertyHandler(new UiScriptingEdit());
    registerPropertyHandler(new UiCustomIcon());
    registerPropertyHandler(new UiUrlButton());
}

QWidget* UiLoader::createWidget(const QString& className, QWidget* parent, const QString& name)
{
    QWidget* w;
    if (registeredClasses.contains(className))
        w = registeredClasses[className](parent, name);
    else
        w = QUiLoader::createWidget(className, parent, name);

    return w;
}

void UiLoader::registerWidgetClass(const QString& className, FactoryFunction factoryFunction)
{
    registeredClasses[className] = factoryFunction;
}

void UiLoader::handlePropertiesRecursively(QWidget* widget)
{
    if (widget->dynamicPropertyNames().size() > 0)
        handleProperties(widget);

    for (QWidget* w : widget->findChildren<QWidget*>())
        handleProperties(w);
}

void UiLoader::handleProperties(QWidget* widget)
{
    QVariant propValue;
    for (UiLoaderPropertyHandler* handler : propertyHandlers)
    {
        propValue = widget->property(handler->getPropertyName());
        if (propValue.isValid())
            handler->handle(widget, propValue);
    }
}

QWidget* UiLoader::load(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "FormManager was unable to open ui file:" << path;
        return nullptr;
    }

    QWidget* w = QUiLoader::load(&file, nullptr);
    handlePropertiesRecursively(w);
    return w;
}

void UiLoader::registerPropertyHandler(UiLoaderPropertyHandler* handler)
{
    propertyHandlers << handler;
}

