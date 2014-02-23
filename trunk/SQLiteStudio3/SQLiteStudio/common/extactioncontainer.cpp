#include "extactioncontainer.h"
#include "iconmanager.h"
#include "common/extaction.h"
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QDebug>

ExtActionContainer::~ExtActionContainer()
{
    deleteActions();
}

void ExtActionContainer::initActions()
{
    createActions();
    setupDefShortcuts();
    refreshShortcuts();
}

void ExtActionContainer::createAction(int action, const QString& icon, const QString& text, const QObject* receiver, const char* slot,
                                      QWidget* container, QWidget* owner)
{
    QAction* qAction = new ExtAction(ICON(icon), text);
    createAction(action, qAction, receiver, slot, container, owner);
}

void ExtActionContainer::createAction(int action, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    QAction* qAction = new ExtAction(text);
    createAction(action, qAction, receiver, slot, container, owner);
}

void ExtActionContainer::defShortcut(int action, int keySequence)
{
    shortcuts[action] = QKeySequence(keySequence).toString();
}

void ExtActionContainer::setShortcutContext(const QList<qint32> actions, Qt::ShortcutContext context)
{
    foreach (qint32 act, actions)
        actionMap[act]->setShortcutContext(context);
}

void ExtActionContainer::attachActionInMenu(int parentAction, int childAction, QToolBar* toolbar)
{
    attachActionInMenu(parentAction, actionMap[childAction], toolbar);
}

void ExtActionContainer::attachActionInMenu(int parentAction, QAction* childAction, QToolBar* toolbar)
{
    QToolButton* button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(actionMap[parentAction]));
    QMenu* menu = button->menu();

    if (!menu)
    {
        menu = new QMenu();
        button->setMenu(menu);
        button->setPopupMode(QToolButton::MenuButtonPopup);
    }

    menu->addAction(childAction);
}

void ExtActionContainer::defShortcutStdKey(int action, QKeySequence::StandardKey standardKey)
{
    shortcuts[action] = QKeySequence(standardKey).toString();
}

void ExtActionContainer::updateShortcutTips()
{
}

void ExtActionContainer::createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    if (!owner)
        owner = container;
    else
        owner->addAction(qAction);

    qAction->setParent(owner);
    actionMap[action] = qAction;
    QObject::connect(qAction, SIGNAL(triggered()), receiver, slot);
    container->addAction(qAction);
}

void ExtActionContainer::deleteActions()
{
    foreach (QAction* action, actionMap.values())
        delete action;

    actionMap.clear();
}

void ExtActionContainer::refreshShortcuts()
{
    foreach (int action, actionMap.keys())
    {
        if (!shortcuts.contains(action))
            continue;

        if (noConfigShortcutActions.contains(action))
            continue;

        actionMap[action]->setShortcut(QKeySequence(shortcuts[action]));
        actionMap[action]->setToolTip(actionMap[action]->text()+QString(" (%1)").arg(shortcuts[action]));
    }

}

QAction* ExtActionContainer::getAction(int action)
{
    if (!actionMap.contains(action))
        return nullptr;

    return actionMap.value(action);
}
