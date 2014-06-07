#ifndef UILOADER_H
#define UILOADER_H

#include <QUiLoader>
#include <QHash>

#define REGISTER_WIDGET(Loader, Class) \
    Loader->registerWidgetClass(#Class, [](QWidget* parent, const QString& name) -> QWidget*\
    {\
        Class* w = new Class(parent);\
        w->setObjectName(name);\
        return w;\
    })

class UiLoader : public QUiLoader
{
        Q_OBJECT
    public:
        typedef std::function<QWidget*(QWidget*, const QString&)> FactoryFunction;

        explicit UiLoader(QObject *parent = 0);

        QWidget* createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString());
        void registerWidgetClass(const QString& className, FactoryFunction factoryFunction);

    private:
        QHash<QString, FactoryFunction> registeredClasses;
};

#endif // UILOADER_H
