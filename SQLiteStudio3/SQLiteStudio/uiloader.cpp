#include "uiloader.h"

UiLoader::UiLoader(QObject *parent) :
    QUiLoader(parent)
{
}

QWidget* UiLoader::createWidget(const QString& className, QWidget* parent, const QString& name)
{
    if (registeredClasses.contains(className))
        return registeredClasses[className](parent, name);

    return QUiLoader::createWidget(className, parent, name);
}

void UiLoader::registerWidgetClass(const QString& className, FactoryFunction factoryFunction)
{
    registeredClasses[className] = factoryFunction;
}
