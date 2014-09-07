#include "extactionprototype.h"
#include "extactioncontainer.h"
#include <QAction>
#include <QDebug>

ExtActionPrototype::ExtActionPrototype(QObject* parent) :
    QObject(parent)
{
    separator = true;
}

ExtActionPrototype::ExtActionPrototype(const QString& text, QObject* parent) :
    QObject(parent), actionText(text)
{
}

ExtActionPrototype::ExtActionPrototype(const QIcon& icon, const QString& text, QObject* parent) :
    QObject(parent), icon(icon), actionText(text)
{
}

ExtActionPrototype::~ExtActionPrototype()
{

}

QString ExtActionPrototype::text() const
{
    return actionText;
}

QAction* ExtActionPrototype::create(QObject* parent)
{
    if (!parent)
        parent = this;

    if (separator)
    {
        QAction* act = new QAction(parent);
        act->setSeparator(true);
        return act;
    }

    return new QAction(icon, actionText, parent);
}

void ExtActionPrototype::emitInsertedTo(ExtActionContainer* actionContainer, int toolbar, QAction* action)
{
    emit insertedTo(actionContainer, toolbar, action);
}

void ExtActionPrototype::emitAboutToRemoveFrom(ExtActionContainer* actionContainer, int toolbar, QAction* action)
{
    emit aboutToRemoveFrom(actionContainer, toolbar, action);
}

void ExtActionPrototype::emitRemovedFrom(ExtActionContainer* actionContainer, int toolbar, QAction* action)
{
    emit removedFrom(actionContainer, toolbar, action);
}

void ExtActionPrototype::emitTriggered(ExtActionContainer* actionContainer, int toolbar)
{
    emit triggered(actionContainer, toolbar);
}
